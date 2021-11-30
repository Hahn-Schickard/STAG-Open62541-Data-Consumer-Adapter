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
  EXPECT_EQ(statuscode, UA_STATUSCODE_GOOD)
    << UA_StatusCode_name(statuscode);

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
      parseDeviceGroup(builder,names,spec,
        (group.has_value()
          ? builder.addDeviceElementGroup(group.value(), name, description)
          : builder.addDeviceElementGroup(name, description)));
      break;
    case 'R':
      (group.has_value()
        ? builder.addReadableMetric(group.value(), name, description,
          Information_Model::DataType::BOOLEAN,
          []()->Information_Model::DataVariant{ return false; })
        : builder.addReadableMetric(name, description,
          Information_Model::DataType::BOOLEAN,
          []()->Information_Model::DataVariant{ return false; }));
      break;
    case 'W':
      (group.has_value()
        ? builder.addWritableMetric(group.value(), name, description,
          Information_Model::DataType::BOOLEAN,
          []()->Information_Model::DataVariant{ return false; },
          [](Information_Model::DataVariant){})
        : builder.addWritableMetric(name, description,
          Information_Model::DataType::BOOLEAN,
          []()->Information_Model::DataVariant{ return false; },
          [](Information_Model::DataVariant){}));
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

  builder.buildDeviceBase(
    names.ref_id(), names.name(), names.description());
  parseDeviceGroup(builder, names, spec, std::optional<std::string>());

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
    @brief Marks all elements as expected
  */
  void expect_all() {
    unexpected.clear();
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

  NodeBuilderTests()
    : server(std::make_shared<Open62541Server>()),
      ua_server(server->getServer()),
      node_builder(server)
  {}

  void compareNamedElement(
    const UA_ReferenceDescription & ref_desc,
    Information_Model::NamedElementPtr element)
  {
    SCOPED_TRACE("NamedElement " + to_string(ua_server,ref_desc));
    ASSERT_TRUE(element) << "null pointer";

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

  void compareDeviceElementGroup(
    const UA_ReferenceDescription & ref_desc,
    Information_Model::DeviceElementGroupPtr group)
  {
    ASSERT_TRUE(group) << "null pointer";
    SCOPED_TRACE("DeviceElementGroup " + to_string(ua_server,ref_desc));
    compareNamedElement(ref_desc, group);

    // check children
    Browse ua_elements(ua_server, ref_desc.nodeId.nodeId);
    EXPECT_EQ(ua_elements.num_children(), group->getSubelements().size());
    auto im_elements = group->getSubelements();
    for (auto & im_element : im_elements) {
      ASSERT_TRUE(im_element) << "null pointer";
      ua_elements.expect(im_element->getElementName(),
        [](const UA_ReferenceDescription & elem_ref)->bool {
          return true;
        },
        [](const UA_ReferenceDescription & elem_ref) {
          ADD_FAILURE() << "TODO (" << to_string(elem_ref.nodeId) << ")";
        });
    }
//    ua_elements.expect_all(); // accept additional elements on UA side
  }

  void compareDevice(
    const UA_ReferenceDescription & ref_desc,
    Information_Model::DevicePtr device)
  {
    SCOPED_TRACE("Device " + to_string(ua_server,ref_desc)
      + toString(&ref_desc.nodeId.nodeId));
    compareNamedElement(ref_desc, device);

    EXPECT_EQ(ref_desc.nodeId.namespaceUri.length, 0);
    EXPECT_EQ(ref_desc.nodeId.serverIndex, 0);
    EXPECT_EQ(ref_desc.browseName.namespaceIndex, 1);

    EXPECT_EQ(ref_desc.nodeClass, UA_NODECLASS_OBJECT);

    // check child(ren)
    Browse device_children(ua_server, ref_desc.nodeId.nodeId);
    device_children.expect(
      "device group",
      [](const UA_ReferenceDescription &)->bool {
        return true;
        // we expect only one element (the group), hence accept whatever
      },
      [&](const UA_ReferenceDescription & group_ref) {
        compareDeviceElementGroup(group_ref, device->getDeviceElementGroup());
      });

    ADD_FAILURE() << "TODO?: check typeDefinition";
  }

  void testAddDeviceNode(Information_Model::DevicePtr device) {
    Browse root_before(ua_server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER));
    root_before.expect_all();

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
    "(RW)", "(RRRWRWWW)",
    "((((((((R))))))))", "((R)((W))((R)((W)))((R)((W))((R)((W)))))",
    "((((R)(R))((R)(W)))(((W)(R))((W)(W))))"
    ));

} // namespace
