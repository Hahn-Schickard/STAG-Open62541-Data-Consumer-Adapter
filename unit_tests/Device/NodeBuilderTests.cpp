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
      + "(" + toString(&type_name) + "); "
    + "isForward=" + (ref.isForward ? "true" : "false") + "; "
    + "nodeId=" + to_string(ref.nodeId) + "; "
    + "browseName=\"" + toString(&ref.browseName) + "\"; "
    + "nodeClass=" + std::to_string(ref.nodeClass) + "; "
    + "typeDefinition=" + to_string(ref.typeDefinition)
      + "(" + typedef_name + ")}";
}

/**
 * @brief Helper class for relating Information_Model::DataType to UA_DataType
 *
 * Specializations of this class are used as parameter types for tests.
 */
template <Information_Model::DataType im, size_t ua>
struct DataTypes {
  static constexpr Information_Model::DataType im_index = im;
  static constexpr size_t ua_index = ua;
  static constexpr const UA_DataType * ua_type = &UA_TYPES[ua_index];
  static Information_Model::DataVariant read() {
    return Information_Model::DataVariant(std::in_place_index<(size_t) im_index>);
  }
  static void write(Information_Model::DataVariant) {}
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

// Helper (semantically a sub-function) for parseDevice below
template <class Types>
void parseDeviceGroup(
  Information_Model::DeviceBuilderInterface & builder,
  NameGenerator & names,
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
      parseDeviceGroup<Types>(builder,names,spec,
        (group.has_value()
          ? builder.addDeviceElementGroup(group.value(), name, description)
          : builder.addDeviceElementGroup(name, description)));
      break;
    case 'R':
      (group.has_value()
        ? builder.addReadableMetric(group.value(), name, description,
          Types::im_index, Types::read)
        : builder.addReadableMetric(name, description,
          Types::im_index, Types::read));
      break;
    case 'W':
      (group.has_value()
        ? builder.addWritableMetric(group.value(), name, description,
          Types::im_index, Types::read, Types::write)
        : builder.addWritableMetric(name, description,
          Types::im_index, Types::read, Types::write));
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
template <class Types>
Information_Model::DevicePtr parseDevice(const char * spec) {
  Information_Model::testing::DeviceMockBuilder builder;
  NameGenerator names;

  builder.buildDeviceBase(
    names.ref_id(), names.name(), names.description());
  parseDeviceGroup<Types>(builder, names, spec, std::optional<std::string>());

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
    iterator(Browse & browse_, size_t index_) : browse(browse_), index(index_) {}
  public:
    bool operator !=(const iterator & other) { return index != other.index; }
    iterator & operator ++() { ++index; return *this; }
    UA_ReferenceDescription & operator *() {
      return browse.result.references[index];
    }
  friend class Browse;
  };

  Browse() = delete;

  /**
   * @brief Initialize with a node's references
   */
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

/**
 * @brief Fixture for testing NodeBuilder::addDevice
 */
template <class Types>
struct NodeBuilderTests : public ::testing::Test {
  std::shared_ptr<Open62541Server> server;
  UA_Server * ua_server;
  NodeBuilder node_builder;

  NodeBuilderTests()
    : server(std::make_shared<Open62541Server>()),
      ua_server(server->getServer()),
      node_builder(server)
  {}

  // For use as a filter predicate
  bool compareId(const UA_NodeId & ua, std::string im) {
    if (ua.identifierType != UA_NODEIDTYPE_STRING)
      return false;
    auto im_ = UA_STRING((char *) im.c_str());
    return UA_String_equal(&ua.identifier.string, &im_);
  }

  void checkReferenceType(
    const UA_ReferenceDescription & ref_desc,
    UA_UInt32 type_id)
  {
    auto expected_type = UA_NODEID_NUMERIC(0, type_id);
    EXPECT_TRUE(UA_NodeId_equal(&ref_desc.referenceTypeId, &expected_type))
      << toString(&ref_desc.referenceTypeId);
  }

  void checkType(UA_NodeId type_node, UA_NodeClass expected_class) {
    UA_NodeClass actual_class;
    auto status = UA_Server_readNodeClass(ua_server, type_node, &actual_class);
    EXPECT_EQ(status, UA_STATUSCODE_GOOD) << UA_StatusCode_name(status);
    EXPECT_EQ(actual_class, expected_class);
  }

  void checkNamedElement(
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

  void checkDeviceElement(
    const UA_ReferenceDescription & ref_desc,
    Information_Model::DeviceElementPtr element)
  {
    SCOPED_TRACE("DeviceElement " + to_string(ua_server,ref_desc));
    checkNamedElement(ref_desc, element);
    checkReferenceType(ref_desc, UA_NS0ID_HASCOMPONENT);
    Browse children(ua_server, ref_desc.nodeId.nodeId);

    switch (element->getElementType()) {
    case Information_Model::GROUP:
      EXPECT_EQ(ref_desc.nodeClass, UA_NODECLASS_OBJECT);
      checkType(ref_desc.typeDefinition.nodeId, UA_NODECLASS_OBJECTTYPE);
      children.ignore([](const UA_ReferenceDescription & child)->bool{
        return (child.nodeClass == UA_NODECLASS_OBJECTTYPE);
      });

      checkDeviceElementGroup(ref_desc, children,
        std::dynamic_pointer_cast<Information_Model::DeviceElementGroup>(
          element));
      break;

    case Information_Model::READABLE: {
      EXPECT_EQ(ref_desc.nodeClass, UA_NODECLASS_VARIABLE);
      checkType(ref_desc.typeDefinition.nodeId, UA_NODECLASS_VARIABLETYPE);
      children.ignore([](const UA_ReferenceDescription & child)->bool{
        return (child.nodeClass == UA_NODECLASS_VARIABLETYPE);
      });

      UA_Variant value;
      auto status =
        UA_Server_readValue(ua_server, ref_desc.nodeId.nodeId, &value);
      EXPECT_EQ(status, UA_STATUSCODE_GOOD) << UA_StatusCode_name(status);
      auto metric =
        std::dynamic_pointer_cast<Information_Model::Metric>(element);
      EXPECT_EQ(value.type, Types::ua_type);
    } break;

    case Information_Model::WRITABLE: {
      EXPECT_EQ(ref_desc.nodeClass, UA_NODECLASS_VARIABLE);
      checkType(ref_desc.typeDefinition.nodeId, UA_NODECLASS_VARIABLETYPE);
      children.ignore([](const UA_ReferenceDescription & child)->bool{
        return (child.nodeClass == UA_NODECLASS_VARIABLETYPE);
      });

      UA_Variant value;
      auto status =
        UA_Server_readValue(ua_server, ref_desc.nodeId.nodeId, &value);
      EXPECT_EQ(status, UA_STATUSCODE_GOOD) << UA_StatusCode_name(status);
      auto writable_metric
        = std::dynamic_pointer_cast<Information_Model::WritableMetric>(element);
      EXPECT_EQ(value.type, Types::ua_type);

      status = UA_Server_writeValue(ua_server, ref_desc.nodeId.nodeId, value);
      EXPECT_EQ(status, UA_STATUSCODE_GOOD) << UA_StatusCode_name(status);
      } break;

    default:
      ADD_FAILURE() << element->getElementType();
    }
  }

  /**
   * All aspects of DeviceElementGroup that are inherited from DeviceElement
   * are ignored. Only the subelements are checked. This is in contrast to the
   * other check* methods, each of which checks the full respective state.
   */
  /*
   * In the overall procedure, the remaining aspects of DeviceElementGroup are
   * checked as part of checkDeviceElement, if the  DeviceElementGroup appears
   * as a specialization of a DeviceElement. If, on the other hand, the
   * DeviceElementGroup appears as the result of Device::getDeviceElementGroup,
   * then there is nothing on the UA side to check against the missing aspects.
   */
  void checkDeviceElementGroup(
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
          checkDeviceElement(elem_ref,im_element);
        });
    }
  }

  void checkDevice(
    const UA_ReferenceDescription & ref_desc,
    Information_Model::DevicePtr device)
  {
    SCOPED_TRACE("Device " + to_string(ua_server,ref_desc)
      + toString(&ref_desc.nodeId.nodeId));
    checkNamedElement(ref_desc, device);
    checkReferenceType(ref_desc, UA_NS0ID_ORGANIZES);
    Browse children(ua_server, ref_desc.nodeId.nodeId);
    children.ignore([](const UA_ReferenceDescription & child)->bool{
      return (child.nodeClass == UA_NODECLASS_OBJECTTYPE);
    });

    EXPECT_EQ(ref_desc.nodeId.namespaceUri.length, 0);
    EXPECT_EQ(ref_desc.nodeId.serverIndex, 0);
    EXPECT_EQ(ref_desc.browseName.namespaceIndex, 1);

    EXPECT_EQ(ref_desc.nodeClass, UA_NODECLASS_OBJECT);
    checkType(ref_desc.typeDefinition.nodeId, UA_NODECLASS_OBJECTTYPE);

    // check child(ren)
    checkDeviceElementGroup(
      ref_desc, children, device->getDeviceElementGroup());
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
        checkDevice(ref_desc, device);
      });
  }

  void testAddDeviceNode(const char * spec) {
    testAddDeviceNode(parseDevice<Types>(spec));
  }
};

using BooleanNodeBuilderTests = NodeBuilderTests
  <DataTypes<Information_Model::DataType::BOOLEAN, UA_TYPES_BOOLEAN>>;

TEST_F(BooleanNodeBuilderTests, fixtureWorksByItself) {}

TEST_F(BooleanNodeBuilderTests, addMockDeviceNode) {
  auto device = std::make_shared<Information_Model::testing::MockDevice>(
    "the_ref_id", "the name", "the description");
  testAddDeviceNode(device);
}

/**
 * @brief Test addDeviceNode with differently structured Device
 */
class AddDeviceNodeParameterizedTest
  : public BooleanNodeBuilderTests,
    public ::testing::WithParamInterface<const char *>
{};
TEST_P(AddDeviceNodeParameterizedTest, addDeviceNodeStructures) {
  testAddDeviceNode(GetParam());
}
INSTANTIATE_TEST_SUITE_P(NodeBuilderAddDeviceNodeParameterizedTestSuite,
  AddDeviceNodeParameterizedTest,
  ::testing::Values(
    "()",
    "(R)", "(W)",
    "(RRWWWWRWWRRRRWRW)",
    "((((((((R))))))))", "((R)((W))((R)((W)))((R)((W))((R)((W)))))",
    "((((R)(R))((R)(W)))(((W)(R))((W)(W))))"
    ));

/**
 * @brief Test addDeviceNode with differently typed variables
 */
using AllTypes = ::testing::Types<
  DataTypes<Information_Model::DataType::BOOLEAN, UA_TYPES_BOOLEAN>,
  DataTypes<Information_Model::DataType::INTEGER, UA_TYPES_INT64>,
  DataTypes<Information_Model::DataType::UNSIGNED_INTEGER, UA_TYPES_UINT64>,
  DataTypes<Information_Model::DataType::DOUBLE, UA_TYPES_DOUBLE>,
  DataTypes<Information_Model::DataType::TIME, UA_TYPES_DATETIME>,
  DataTypes<Information_Model::DataType::OPAQUE, UA_TYPES_BYTESTRING>,
  DataTypes<Information_Model::DataType::STRING, UA_TYPES_STRING>
  >;
TYPED_TEST_SUITE(NodeBuilderTests, AllTypes);
TYPED_TEST(NodeBuilderTests, addDeviceNodeTypes) {
  TestFixture::testAddDeviceNode("(RW)");
}

} // namespace
