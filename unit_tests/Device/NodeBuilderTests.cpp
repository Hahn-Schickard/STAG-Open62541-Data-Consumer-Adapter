#include <functional>

#include "gtest/gtest.h"

#include "Information_Model/mocks/DeviceMockBuilder.hpp"
#include "NodeBuilder.hpp"
#include "Utility.hpp"

namespace NodeBuilderTests {

using namespace open62541;

std::string to_string(const UA_ExpandedNodeId & id) {
  return std::to_string(id.serverIndex) + ":" + toString(&id.namespaceUri)
    + ":" + toString(&id.nodeId);
}

std::string to_string(UA_Server * server, const UA_ReferenceDescription & ref) {
  UA_QualifiedName type_name;
  auto statuscode =
    UA_Server_readBrowseName(server, ref.referenceTypeId, &type_name);
  EXPECT_EQ(statuscode, UA_STATUSCODE_GOOD) << UA_StatusCode_name(statuscode);

  std::string typedef_name;
  {
    UA_QualifiedName typedef_name_;
    statuscode = UA_Server_readBrowseName(
      server, ref.typeDefinition.nodeId, &typedef_name_);
    switch (statuscode) {
    case UA_STATUSCODE_GOOD:
      typedef_name = toString(&typedef_name_);
      break;
    case UA_STATUSCODE_BADNODEIDUNKNOWN:
      typedef_name = "?";
      break;
    default:
      ADD_FAILURE() << UA_StatusCode_name(statuscode);
    }
  }

  return "{referenceTypeID=" + toString(&ref.referenceTypeId)
      + "(" + toString(&type_name)
    + "); isForward=" + (ref.isForward ? "true" : "false")
    + "; nodeId=" + to_string(ref.nodeId)
    + "; browseName=\"" + toString(&ref.browseName)
    + "\"; nodeClass=" + std::to_string(ref.nodeClass)
    + "; typeDefinition=" + to_string(ref.typeDefinition) + "(" + typedef_name
    + ")}";
}

/**
 * @brief Helper class for relating Information_Model::DataType to UA_DataType
 */
class Types {

  std::vector<Information_Model::DataType> im_types;
  std::map<Information_Model::DataType, const UA_DataType *> ua_types;
  std::map<Information_Model::DataType, Information_Model::ReadFunctor> reads;

  template <Information_Model::DataType im>
  void add_type(size_t ua) {
    im_types.push_back(im);
    ua_types.insert(std::pair(im, &UA_TYPES[ua]));
    reads.insert(std::pair(im, []()->Information_Model::DataVariant{
      return Information_Model::DataVariant(std::in_place_index<(size_t) im>);
    }));
  }

public:
  class iterator {
    size_t i = 0;
    Types & types;
  public:
    iterator(Types & types_) : types(types_) {}

    void operator ++() { ++i; }
    Information_Model::DataType operator *() {
      return types.im_types[i % types.im_types.size()];
    }
  };

  Types() {
    add_type<Information_Model::DataType::BOOLEAN>(UA_TYPES_BOOLEAN);
    add_type<Information_Model::DataType::INTEGER>(UA_TYPES_INT64);
    add_type<Information_Model::DataType::UNSIGNED_INTEGER>(UA_TYPES_UINT64);
    add_type<Information_Model::DataType::DOUBLE>(UA_TYPES_DOUBLE);
    add_type<Information_Model::DataType::TIME>(UA_TYPES_DATETIME);
    add_type<Information_Model::DataType::OPAQUE>(UA_TYPES_BYTESTRING);
    add_type<Information_Model::DataType::STRING>(UA_TYPES_STRING);
  }

  iterator begin() {
    return iterator(*this);
  }

  const UA_DataType * ua_type(Information_Model::DataType im) {
    return ua_types[im];
  }

  Information_Model::ReadFunctor read(Information_Model::DataType im) {
    return reads[im];
  }
};

/**
 * @brief Helper class for inventing Information_Model::NamedElement data
 */
class NameGenerator {
  size_t counter = 0;
  std::string make(const char * prefix) const {
    return std::string(prefix) + std::to_string(counter);
  }
public:
  std::string ref_id() const { return make("ref_id_"); }
  std::string name() const { return make("name "); }
  std::string description() const { return make("description "); }
  void next() { ++counter; }
};

// helper for parseDevice below
void parseDeviceGroup(
  Information_Model::DeviceBuilderInterface & builder,
  NameGenerator & names,
  Types & types,
  Types::iterator & r_type, Types::iterator & w_type,
  const char *& spec,
  std::optional<std::string> group)
{
  // precondition: *spec == '('
  // postcondition: *spec == ')'

  ++spec;
  while (*spec != ')') {
    names.next();
    auto name = names.name();
    auto description = names.description();

    switch(*spec) {
    case '(':
      parseDeviceGroup(builder,names,types,r_type,w_type,spec,
        (group.has_value()
          ? builder.addDeviceElementGroup(group.value(), name, description)
          : builder.addDeviceElementGroup(name, description)));
      break;
    case 'R':
      (group.has_value()
        ? builder.addReadableMetric(group.value(), name, description,
          *r_type, types.read(*r_type))
        : builder.addReadableMetric(name, description,
          *r_type, types.read(*r_type)));
      ++r_type;
      break;
    case 'W':
      (group.has_value()
        ? builder.addWritableMetric(group.value(), name, description,
          *w_type, types.read(*w_type),
          [](Information_Model::DataVariant){})
        : builder.addWritableMetric(name, description,
          *w_type, types.read(*w_type),
          [](Information_Model::DataVariant){}));
      ++w_type;
      break;
    default:
      throw "illegal spec";
    }

    ++spec;
  }
}

/**
 * @brief Create a Device from a string spec.
 *
 * The spec format is:
 * - "R" for a READABLE element
 * - "W" for a WRITABLE element
 * - For a GROUP element: The concatenation of its children's specs enclosed
 *   between "(" and ")"
 * - For the Device: Its ElementGroup's spec
 */
Information_Model::DevicePtr parseDevice(const char * spec) {
  Information_Model::testing::DeviceMockBuilder builder;
  NameGenerator names;
  Types types;
  auto r_type = types.begin();
  auto w_type = types.begin();

  builder.buildDeviceBase(
    names.ref_id(), names.name(), names.description());
  parseDeviceGroup(
    builder, names, types, r_type, w_type, spec, std::optional<std::string>());

  return builder.getResult();
}

/**
 * @brief Helper class for comparing browsed with expected state
 *
 * The destructor fails if not all browsed elements have been marked as
 * expected until then.
 */
class Browse {
  static constexpr size_t max_references = 1000;

  UA_Server * ua_server;
  UA_BrowseResult result;
  std::set<size_t> unexpected;
  size_t num_children_;

public:
  class iterator {
    Browse & browse;
    size_t index;
  public:
    iterator(Browse & browse_, size_t index_) : browse(browse_), index(index_) {}
    bool operator !=(const iterator & other) { return index != other.index; }
    iterator & operator ++() { ++index; return *this; }
    UA_ReferenceDescription & operator *() {
      return browse.result.references[index];
    }
  };

  Browse() = delete;

  Browse(UA_Server * ua_server_, const UA_NodeId & node_id)
    : ua_server(ua_server_)
  {
    UA_BrowseDescription browse_description;
    browse_description.nodeId = node_id;
    browse_description.browseDirection = UA_BROWSEDIRECTION_FORWARD;
    browse_description.referenceTypeId = UA_NODEID_NULL;
    browse_description.includeSubtypes = UA_TRUE;
    browse_description.nodeClassMask = 65535;
    browse_description.resultMask = 65535;
    result = UA_Server_browse(ua_server, max_references, &browse_description);
    EXPECT_EQ(result.statusCode, UA_STATUSCODE_GOOD)
      << UA_StatusCode_name(result.statusCode);
    EXPECT_LT(result.referencesSize, max_references);

    num_children_ = result.referencesSize;

    for (size_t i=0; i<num_children_; ++i)
      unexpected.insert(i);
  }

  ~Browse() {
    for (auto i : unexpected) {
      auto & ref = result.references[i];
      ADD_FAILURE()
        << "unexpected reference " << to_string(ua_server,ref);
    }
  }

  iterator begin() { return iterator(*this, 0); }
  iterator end() { return iterator(*this, result.referencesSize); }

  /*
    @brief Marks some elements as expected
  */
  void ignore(std::function<bool (const UA_ReferenceDescription &)> filter) {
    auto i = unexpected.begin();
    while (i != unexpected.end()) {
      auto next = i; ++next;
      if (filter(result.references[*i]))
        unexpected.erase(i);
      i = next;
    }
  }

  /*
    @brief Marks one element as expected and tests it further
  */
  void expect(
    std::string filter_description,
    std::function<bool (const UA_ReferenceDescription &)> filter,
    std::function<void (const UA_ReferenceDescription &)> test)
  {
    for (auto i : unexpected)
      if (filter(result.references[i])) {
        unexpected.erase(i);
        test(result.references[i]);
        return;
      }
    ADD_FAILURE() << filter_description << " not found";
  }

  size_t num_children() const {
    return num_children_;
  }
};


struct NodeBuilderTests : public ::testing::Test {
  std::shared_ptr<Open62541Server> server;
  UA_Server * ua_server;
  NodeBuilder node_builder;
  Types types;

  NodeBuilderTests()
    : server(std::make_shared<Open62541Server>()),
      ua_server(server->getServer()),
      node_builder(server)
  {}

  bool compareId(const UA_NodeId & ua, std::string im) {
    if (ua.identifierType != UA_NODEIDTYPE_STRING)
      return false;
    auto im_ = UA_STRING((char *) im.c_str());
    return UA_String_equal(&ua.identifier.string, &im_);
  }

  void compareType(UA_NodeId type_node, UA_NodeClass expected_class) {
    UA_NodeClass actual_class;
    auto status = UA_Server_readNodeClass(ua_server, type_node, &actual_class);
    EXPECT_EQ(status, UA_STATUSCODE_GOOD) << UA_StatusCode_name(status);
    EXPECT_EQ(actual_class, expected_class);
  }

  void compareNamedElement(
    const UA_ReferenceDescription & ref_desc,
    Information_Model::NamedElementPtr element)
  {
    SCOPED_TRACE("NamedElement " + to_string(ua_server,ref_desc));
    ASSERT_TRUE(element) << "null pointer";

    EXPECT_TRUE(ref_desc.isForward);

    // check browseName
    auto expected_browse_name
      = UA_String_fromChars(element->getElementName().c_str());
    EXPECT_TRUE(UA_String_equal(
        &ref_desc.browseName.name, &expected_browse_name))
      << toString(&ref_desc.browseName.name)
      << " vs " << toString(&expected_browse_name);

    // check displayName
    auto expected_display_name
      = UA_String_fromChars(element->getElementName().c_str());
    EXPECT_TRUE(UA_String_equal(
        &ref_desc.displayName.text, &expected_display_name))
      << toString(&ref_desc.displayName.text)
      << " vs " << toString(&expected_display_name);
  }

  void compareDeviceElement(
    const UA_ReferenceDescription & ref_desc,
    Information_Model::DeviceElementPtr element)
  {
    SCOPED_TRACE("DeviceElement " + to_string(ua_server,ref_desc));
    compareNamedElement(ref_desc, element);
    Browse children(ua_server, ref_desc.nodeId.nodeId);

    switch (element->getElementType()) {
    case Information_Model::GROUP:
      EXPECT_EQ(ref_desc.nodeClass, UA_NODECLASS_OBJECT);
      compareType(ref_desc.typeDefinition.nodeId, UA_NODECLASS_OBJECTTYPE);
      children.ignore([](const UA_ReferenceDescription & child)->bool{
        return (child.nodeClass == UA_NODECLASS_OBJECTTYPE);
      });

      compareDeviceElementGroup(ref_desc, children,
        std::dynamic_pointer_cast<Information_Model::DeviceElementGroup>(
          element));
      break;

    case Information_Model::READABLE: {
      EXPECT_EQ(ref_desc.nodeClass, UA_NODECLASS_VARIABLE);
      compareType(ref_desc.typeDefinition.nodeId, UA_NODECLASS_VARIABLETYPE);
      children.ignore([](const UA_ReferenceDescription & child)->bool{
        return (child.nodeClass == UA_NODECLASS_VARIABLETYPE);
      });

      UA_Variant value;
      auto status =
        UA_Server_readValue(ua_server, ref_desc.nodeId.nodeId, &value);
      EXPECT_EQ(status, UA_STATUSCODE_GOOD) << UA_StatusCode_name(status);
      auto metric =
        std::dynamic_pointer_cast<Information_Model::Metric>(element);
      EXPECT_EQ(value.type, types.ua_type(metric->getDataType()));
    } break;

    case Information_Model::WRITABLE: {
      EXPECT_EQ(ref_desc.nodeClass, UA_NODECLASS_VARIABLE);
      compareType(ref_desc.typeDefinition.nodeId, UA_NODECLASS_VARIABLETYPE);
      children.ignore([](const UA_ReferenceDescription & child)->bool{
        return (child.nodeClass == UA_NODECLASS_VARIABLETYPE);
      });

      UA_Variant value;
      auto status =
        UA_Server_readValue(ua_server, ref_desc.nodeId.nodeId, &value);
      EXPECT_EQ(status, UA_STATUSCODE_GOOD) << UA_StatusCode_name(status);
      auto writable_metric
        = std::dynamic_pointer_cast<Information_Model::WritableMetric>(element);
      EXPECT_EQ(value.type, types.ua_type(writable_metric->getDataType()));

      status = UA_Server_writeValue(ua_server, ref_desc.nodeId.nodeId, value);
      EXPECT_EQ(status, UA_STATUSCODE_GOOD) << UA_StatusCode_name(status);
      } break;

    default:
      ADD_FAILURE() << element->getElementType();
    }

    ADD_FAILURE() << "TODO: check referenceTypeId";
  }

  void compareDeviceElementGroup(
    const UA_ReferenceDescription & ref_desc,
    Browse & children,
    Information_Model::DeviceElementGroupPtr group)
  {
    SCOPED_TRACE("DeviceElementGroup " + to_string(ua_server,ref_desc));

    // check children
    auto im_elements = group->getSubelements();
    for (auto & im_element : im_elements) {
      ASSERT_TRUE(im_element) << "null pointer";
      children.expect(im_element->getElementName(),
        [&](const UA_ReferenceDescription & elem_ref)->bool {
          return compareId(elem_ref.nodeId.nodeId, im_element->getElementId());
        },
        [&](const UA_ReferenceDescription & elem_ref) {
          compareDeviceElement(elem_ref,im_element);
        });
    }
  }

  void compareDevice(
    const UA_ReferenceDescription & ref_desc,
    Information_Model::DevicePtr device)
  {
    SCOPED_TRACE("Device " + to_string(ua_server,ref_desc)
      + toString(&ref_desc.nodeId.nodeId));
    compareNamedElement(ref_desc, device);
    Browse children(ua_server, ref_desc.nodeId.nodeId);
    children.ignore([](const UA_ReferenceDescription & child)->bool{
      return (child.nodeClass == UA_NODECLASS_OBJECTTYPE);
    });

    EXPECT_EQ(ref_desc.nodeId.namespaceUri.length, 0);
    EXPECT_EQ(ref_desc.nodeId.serverIndex, 0);
    EXPECT_EQ(ref_desc.browseName.namespaceIndex, 1);

    EXPECT_EQ(ref_desc.nodeClass, UA_NODECLASS_OBJECT);
    compareType(ref_desc.typeDefinition.nodeId, UA_NODECLASS_OBJECTTYPE);

    // check child(ren)
    compareDeviceElementGroup(
      ref_desc, children, device->getDeviceElementGroup());

    ADD_FAILURE() << "TODO: check referenceTypeId";
  }

  void testAddDeviceNode(Information_Model::DevicePtr device) {
    Browse root_before(ua_server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER));
    root_before.ignore([](const UA_ReferenceDescription &)->bool {return true;});

    // Make the call
    auto status = node_builder.addDeviceNode(device);
    EXPECT_EQ(status, UA_STATUSCODE_GOOD) << UA_StatusCode_name(status);

    Browse root_after(ua_server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER));

    // Move everything from root_before out of the way
    for (auto old_ref : root_before)
      root_after.expect(
        to_string(old_ref.nodeId) + " (old)",
        [&](const UA_ReferenceDescription & ref_desc)->bool {
          return UA_ExpandedNodeId_equal(&ref_desc.nodeId, &old_ref.nodeId);
        },
        [](const UA_ReferenceDescription &){});

    auto expected_id
      = UA_NODEID_STRING(1, (char *) device->getElementId().c_str());
    root_after.expect(
      toString(&expected_id),
      [&](const UA_ReferenceDescription & ref_desc)->bool {
        return UA_NodeId_equal(&ref_desc.nodeId.nodeId, &expected_id);
      },
      [&](const UA_ReferenceDescription & ref_desc) {
        compareDevice(ref_desc, device);
      });
  }

  void testAddDeviceNode(const char * spec) {
    testAddDeviceNode(parseDevice(spec));
  }
};

TEST_F(NodeBuilderTests, fixtureWorksByItself) {
}

TEST_F(NodeBuilderTests, addMockDeviceNode) {
  auto device = std::make_shared<Information_Model::testing::MockDevice>(
    "the_ref_id", "the name", "the description");
  testAddDeviceNode(device);
}

class AddDeviceNodeTest
  : public NodeBuilderTests,
    public ::testing::WithParamInterface<const char *>
{};
TEST_P(AddDeviceNodeTest, addDeviceNode) {
  testAddDeviceNode(GetParam());
}
INSTANTIATE_TEST_SUITE_P(NodeBuilderAddDeviceNodeTestSuite, AddDeviceNodeTest,
  ::testing::Values(
    "()",
    "(R)", "(W)",
    "(RW)", "(RRWWWWRWWRRRRWRW)",
    "((((((((R))))))))", "((R)((W))((R)((W)))((R)((W))((R)((W)))))",
    "((((R)(R))((R)(W)))(((W)(R))((W)(W))))"
    ));

} // namespace
