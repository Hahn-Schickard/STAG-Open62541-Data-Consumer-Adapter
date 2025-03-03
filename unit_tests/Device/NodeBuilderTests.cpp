#include <functional>

#include "gtest/gtest.h"

#include "Information_Model/mocks/DeviceMockBuilder.hpp"
#include "NodeBuilder.hpp"
#include "Utility.hpp"

using namespace open62541;

std::string toString(UA_Server* server, const UA_ReferenceDescription& ref) {
  UA_QualifiedName ua_type_name;
  auto statuscode =
      UA_Server_readBrowseName(server, ref.referenceTypeId, &ua_type_name);
  EXPECT_EQ(statuscode, UA_STATUSCODE_GOOD) << UA_StatusCode_name(statuscode);
  std::string type_name = toString(&ua_type_name);
  UA_QualifiedName_clear(&ua_type_name);

  std::string typedef_name;
  {
    UA_QualifiedName opcua_typedef_name;
    statuscode = UA_Server_readBrowseName(
        server, ref.typeDefinition.nodeId, &opcua_typedef_name);
    switch (statuscode) {
    case UA_STATUSCODE_GOOD:
      typedef_name = toString(&opcua_typedef_name);
      break;
    case UA_STATUSCODE_BADNODEIDUNKNOWN:
      typedef_name = "?";
      break;
    default:
      ADD_FAILURE() << UA_StatusCode_name(statuscode);
    }
    UA_QualifiedName_clear(&opcua_typedef_name);
  }

  return "{referenceTypeID=" + toString(&ref.referenceTypeId) + "(" +
      type_name + "); " + //
      "isForward=" + (ref.isForward ? "true" : "false") + "; " +
      "nodeId=" + toString(ref.nodeId) + "; " + "browseName=\"" +
      toString(&ref.browseName) + "\"; " +
      "nodeClass=" + std::to_string(ref.nodeClass) + "; " +
      "typeDefinition=" + toString(ref.typeDefinition) + "(" + typedef_name +
      ")}";
}

/**
 * @brief Helper class for relating Information_Model::DataType to UA_DataType
 *
 * Specializations of this class are used as parameter types for tests.
 */
template <Information_Model::DataType im, size_t ua> struct DataTypes {
  static constexpr Information_Model::DataType IM_INDEX = im;
  static constexpr size_t UA_INDEX = ua;
  static constexpr const UA_DataType* UA_TYPE = &UA_TYPES[UA_INDEX];
  static Information_Model::DataVariant read() {
    return Information_Model::DataVariant(
        std::in_place_index<(size_t)IM_INDEX>);
  }
  static void write(const Information_Model::DataVariant&) {}
};

/**
 * @brief Helper class for inventing Information_Model::NamedElement data
 */
class NameGenerator {
  size_t counter_ = 0;
  std::string make(const char* prefix) const {
    return std::string(prefix) + std::to_string(counter_);
  }

public:
  std::string refId() const { return make("ref_id_"); }
  std::string name() const { return make("name "); }
  std::string description() const { return make("description "); }
  void next() { ++counter_; }
};

// Helper (semantically a sub-function) for parseDevice below
template <class Types>
void parseDeviceGroup(Information_Model::DeviceBuilderInterface& builder,
    NameGenerator& names, const char*& spec, std::optional<std::string> group) {
  // precondition: *spec == '('
  // postcondition: *spec == ')'

  ++spec;
  while (*spec != ')') {
    names.next();
    auto name = names.name();
    auto description = names.description();

    switch (*spec) {
    case '(':
      parseDeviceGroup<Types>(builder, names, spec,
          (group.has_value()
                  ? builder.addDeviceElementGroup(
                        group.value(), name, description)
                  : builder.addDeviceElementGroup(name, description)));
      break;
    case 'R':
      (group.has_value() ? builder.addReadableMetric(group.value(), name,
                               description, Types::IM_INDEX, Types::read)
                         : builder.addReadableMetric(name, description,
                               Types::IM_INDEX, Types::read));
      break;
    case 'W':
      (group.has_value()
              ? builder.addWritableMetric(group.value(), name, description,
                    Types::IM_INDEX, Types::write, Types::read)
              : builder.addWritableMetric(name, description, Types::IM_INDEX,
                    Types::write, Types::read));
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
Information_Model::NonemptyDevicePtr parseDevice(const char* spec) {
  Information_Model::testing::DeviceMockBuilder builder;
  NameGenerator names;

  builder.buildDeviceBase(names.refId(), names.name(), names.description());
  parseDeviceGroup<Types>(builder, names, spec, std::optional<std::string>());

  return Information_Model::NonemptyDevicePtr(builder.getResult());
}

/**
 * @brief Helper class for comparing browsed with expected state
 *
 * The destructor fails if not all browsed elements have been marked as
 * expected until then.
 */
class Browse {
  static constexpr size_t MAX_REFERENCES = 1000;

  UA_Server* ua_server_;
  UA_BrowseResult result_;
  std::set<size_t> unexpected_;
  size_t num_children_;

public:
  class Iterator {
    Browse& browse_;
    size_t index_;
    Iterator(Browse& browse, size_t index) : browse_(browse), index_(index) {}

  public:
    bool operator!=(const Iterator& other) const {
      return index_ != other.index_;
    }
    Iterator& operator++() {
      ++index_;
      return *this;
    }
    UA_ReferenceDescription& operator*() const {
      return browse_.result_.references[index_];
    }
    friend class Browse;
  };

  Browse() = delete;

  /**
   * @brief Initialize with a node's references
   */
  Browse(UA_Server* ua_server, const UA_NodeId& node_id)
      : ua_server_(ua_server) {
    UA_BrowseDescription browse_description;
    browse_description.nodeId = node_id;
    browse_description.browseDirection = UA_BROWSEDIRECTION_FORWARD;
    browse_description.referenceTypeId = UA_NODEID_NULL;
    browse_description.includeSubtypes = UA_TRUE;
    // NOLINTNEXTLINE(readability-magic-numbers)
    browse_description.nodeClassMask = 65535;
    browse_description.resultMask = 65535; // NOLINT(readability-magic-numbers)
    result_ = UA_Server_browse(ua_server_, MAX_REFERENCES, &browse_description);

    EXPECT_EQ(result_.statusCode, UA_STATUSCODE_GOOD)
        << UA_StatusCode_name(result_.statusCode);
    EXPECT_LT(result_.referencesSize, MAX_REFERENCES);

    num_children_ = result_.referencesSize;

    for (size_t i = 0; i < num_children_; ++i) {
      unexpected_.insert(i);
    }
  }

  ~Browse() {
    for (auto i : unexpected_) {
      auto& ref = result_.references[i];
      ADD_FAILURE() << "unexpected reference " << toString(ua_server_, ref);
    }
    UA_BrowseResult_clear(&result_);
  }

  Iterator begin() { return Iterator(*this, 0); }
  Iterator end() { return Iterator(*this, result_.referencesSize); }

  /*
    @brief Marks some elements as expected
  */
  void ignore(
      const std::function<bool(const UA_ReferenceDescription&)>& filter) {
    auto i = unexpected_.begin();
    while (i != unexpected_.end()) {
      auto next = i;
      ++next;
      if (filter(result_.references[*i])) {
        unexpected_.erase(i);
      }
      i = next;
    }
  }

  /*
    @brief Marks one element as expected and tests it further
  */
  void expect(const std::string& filter_description,
      const std::function<bool(const UA_ReferenceDescription&)>& filter,
      const std::function<void(const UA_ReferenceDescription&)>& test) {
    for (auto i : unexpected_) {
      if (filter(result_.references[i])) {
        unexpected_.erase(i);
        test(result_.references[i]);
        return;
      }
    }
    ADD_FAILURE() << filter_description << " not found";
  }

  size_t numChildren() const { return num_children_; }
};

/**
 * @brief Fixture for testing NodeBuilder::addDevice
 */
template <class Types> struct NodeBuilderTests : public ::testing::Test {
  std::shared_ptr<Open62541Server> server;
  UA_Server* ua_server;
  NodeBuilder node_builder;

  NodeBuilderTests()
      : server(std::make_shared<Open62541Server>()),
        ua_server(server->getServer()), node_builder(server) {}

  // For use as a filter predicate
  bool compareId(const UA_NodeId& ua, const std::string& im) {
    if (ua.identifierType != UA_NODEIDTYPE_STRING) {
      return false;
    }
    auto open6254_im = UA_STRING((char*)im.c_str());
    return UA_String_equal(&ua.identifier.string, &open6254_im);
  }

  void checkReferenceType(
      const UA_ReferenceDescription& ref_desc, UA_UInt32 type_id) {
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

  void checkNamedElement(const UA_ReferenceDescription& ref_desc,
      Information_Model::NonemptyNamedElementPtr element) {
    SCOPED_TRACE("NamedElement " + toString(ua_server, ref_desc));

    EXPECT_TRUE(ref_desc.isForward);

    // check browseName
    auto expected_browse_name =
        UA_String_fromChars(element->getElementName().c_str());
    EXPECT_TRUE(
        UA_String_equal(&ref_desc.browseName.name, &expected_browse_name))
        << toString(&ref_desc.browseName.name) << " vs "
        << toString(&expected_browse_name);
    UA_String_clear(&expected_browse_name);

    // check displayName
    auto expected_display_name =
        UA_String_fromChars(element->getElementName().c_str());
    EXPECT_TRUE(
        UA_String_equal(&ref_desc.displayName.text, &expected_display_name))
        << toString(&ref_desc.displayName.text) << " vs "
        << toString(&expected_display_name);
    UA_String_clear(&expected_display_name);

    // check description
    auto expected_description =
        UA_String_fromChars(element->getElementDescription().c_str());
    UA_LocalizedText description;
    auto status = UA_Server_readDescription(
        ua_server, ref_desc.nodeId.nodeId, &description);
    EXPECT_EQ(status, UA_STATUSCODE_GOOD) << UA_StatusCode_name(status);
    EXPECT_TRUE(UA_String_equal(&description.text, &expected_description));
    UA_String_clear(&expected_description);
    UA_LocalizedText_clear(&description);
  }

  // NOLINTNEXTLINE(readability-function-cognitive-complexity)
  void checkDeviceElement(const UA_ReferenceDescription& ref_desc,
      Information_Model::NonemptyDeviceElementPtr element) {
    SCOPED_TRACE("DeviceElement " + toString(ua_server, ref_desc));
    checkNamedElement(ref_desc, element);
    checkReferenceType(ref_desc, UA_NS0ID_HASCOMPONENT);
    Browse children(ua_server, ref_desc.nodeId.nodeId);

    match(
        element->functionality,
        [&](const Information_Model::NonemptyDeviceElementGroupPtr& group) {
          EXPECT_EQ(ref_desc.nodeClass, UA_NODECLASS_OBJECT);
          checkType(ref_desc.typeDefinition.nodeId, UA_NODECLASS_OBJECTTYPE);
          children.ignore([](const UA_ReferenceDescription& child) -> bool {
            return (child.nodeClass == UA_NODECLASS_OBJECTTYPE);
          });

          checkDeviceElementGroup(ref_desc, children, group);
        },
        [&](const Information_Model::NonemptyObservableMetricPtr&) {
          // @TODO: fix this variant so it does not reuse the code
          // ObservableMetrics are treated as Metric instances in OPC UA
          EXPECT_EQ(ref_desc.nodeClass, UA_NODECLASS_VARIABLE);
          checkType(ref_desc.typeDefinition.nodeId, UA_NODECLASS_VARIABLETYPE);
          children.ignore([](const UA_ReferenceDescription& child) -> bool {
            return (child.nodeClass == UA_NODECLASS_VARIABLETYPE);
          });

          UA_Variant value;
          auto status =
              UA_Server_readValue(ua_server, ref_desc.nodeId.nodeId, &value);
          EXPECT_EQ(status, UA_STATUSCODE_GOOD) << UA_StatusCode_name(status);
          EXPECT_EQ(value.type, Types::UA_TYPE);
          UA_Variant_clear(&value);
        },
        [&](const Information_Model::NonemptyMetricPtr&) {
          EXPECT_EQ(ref_desc.nodeClass, UA_NODECLASS_VARIABLE);
          checkType(ref_desc.typeDefinition.nodeId, UA_NODECLASS_VARIABLETYPE);
          children.ignore([](const UA_ReferenceDescription& child) -> bool {
            return (child.nodeClass == UA_NODECLASS_VARIABLETYPE);
          });

          UA_Variant value;
          auto status =
              UA_Server_readValue(ua_server, ref_desc.nodeId.nodeId, &value);
          EXPECT_EQ(status, UA_STATUSCODE_GOOD) << UA_StatusCode_name(status);
          EXPECT_EQ(value.type, Types::UA_TYPE);
          UA_Variant_clear(&value);
        },
        [&](const Information_Model::NonemptyWritableMetricPtr&) {
          EXPECT_EQ(ref_desc.nodeClass, UA_NODECLASS_VARIABLE);
          checkType(ref_desc.typeDefinition.nodeId, UA_NODECLASS_VARIABLETYPE);
          children.ignore([](const UA_ReferenceDescription& child) -> bool {
            return (child.nodeClass == UA_NODECLASS_VARIABLETYPE);
          });

          UA_Variant value;
          auto status =
              UA_Server_readValue(ua_server, ref_desc.nodeId.nodeId, &value);
          EXPECT_EQ(status, UA_STATUSCODE_GOOD) << UA_StatusCode_name(status);
          EXPECT_EQ(value.type, Types::UA_TYPE);

          status =
              UA_Server_writeValue(ua_server, ref_desc.nodeId.nodeId, value);
          EXPECT_EQ(status, UA_STATUSCODE_GOOD) << UA_StatusCode_name(status);
          UA_Variant_clear(&value);
        },
        [&](const Information_Model::NonemptyFunctionPtr&) {
          // @TODO: handle functions here
        });
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
  void checkDeviceElementGroup(const UA_ReferenceDescription& ref_desc,
      Browse& children,
      Information_Model::NonemptyDeviceElementGroupPtr group) {
    SCOPED_TRACE("DeviceElementGroup " + toString(ua_server, ref_desc));

    // check children
    auto im_elements = group->getSubelements();
    for (auto& im_element : im_elements) {
      children.expect(
          im_element->getElementName(),
          [&](const UA_ReferenceDescription& elem_ref) -> bool {
            return compareId(
                elem_ref.nodeId.nodeId, im_element->getElementId());
          },
          [&](const UA_ReferenceDescription& elem_ref) {
            checkDeviceElement(elem_ref, im_element);
          });
    }
  }

  void checkDevice(const UA_ReferenceDescription& ref_desc,
      Information_Model::NonemptyDevicePtr device) {
    SCOPED_TRACE("Device " + toString(ua_server, ref_desc) +
        toString(&ref_desc.nodeId.nodeId));
    checkNamedElement(ref_desc, device);
    checkReferenceType(ref_desc, UA_NS0ID_ORGANIZES);
    Browse children(ua_server, ref_desc.nodeId.nodeId);
    children.ignore([](const UA_ReferenceDescription& child) -> bool {
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

  void testAddDeviceNode(Information_Model::NonemptyDevicePtr device) {
    Browse root_before(ua_server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER));
    root_before.ignore(
        [](const UA_ReferenceDescription&) -> bool { return true; });

    // Make the call
    auto status = node_builder.addDeviceNode(device);
    EXPECT_EQ(status, UA_STATUSCODE_GOOD) << UA_StatusCode_name(status);

    Browse root_after(ua_server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER));

    // Move everything from root_before out of the way
    for (auto old_ref : root_before) {
      root_after.expect(
          toString(old_ref.nodeId) + " (old)",
          [&](const UA_ReferenceDescription& ref_desc) -> bool {
            return UA_ExpandedNodeId_equal(&ref_desc.nodeId, &old_ref.nodeId);
          },
          [](const UA_ReferenceDescription&) {});
    }

    std::string device_id = device->getElementId();
    auto expected_id = UA_NODEID_STRING(1, (char*)device_id.c_str());
    root_after.expect(
        toString(&expected_id),
        [&](const UA_ReferenceDescription& ref_desc) -> bool {
          return UA_NodeId_equal(&ref_desc.nodeId.nodeId, &expected_id);
        },
        [&](const UA_ReferenceDescription& ref_desc) {
          checkDevice(ref_desc, device);
        });
  }

  void testAddDeviceNode(const char* spec) {
    testAddDeviceNode(parseDevice<Types>(spec));
  }
};

using BooleanNodeBuilderTests = NodeBuilderTests<
    DataTypes<Information_Model::DataType::BOOLEAN, UA_TYPES_BOOLEAN>>;

// NOLINTNEXTLINE
TEST_F(BooleanNodeBuilderTests, fixtureWorksByItself) {}

// NOLINTNEXTLINE
TEST_F(BooleanNodeBuilderTests, addMockDeviceNode) {
  auto device = Nonempty::make_shared<
      ::testing::NiceMock<Information_Model::testing::MockDevice>>(
      "the_ref_id", "the name", "the description");
  testAddDeviceNode(device);
}

/**
 * @brief Test addDeviceNode with differently structured Device
 */
class AddDeviceNodeParameterizedTest
    : public BooleanNodeBuilderTests,
      public ::testing::WithParamInterface<const char*> {};

// NOLINTNEXTLINE
TEST_P(AddDeviceNodeParameterizedTest, addDeviceNodeStructures) {
  testAddDeviceNode(GetParam());
}

// NOLINTNEXTLINE
INSTANTIATE_TEST_SUITE_P(NodeBuilderAddDeviceNodeParameterizedTestSuite,
    AddDeviceNodeParameterizedTest,
    ::testing::Values("()", "(R)", "(W)", "(RRWWWWRWWRRRRWRW)",
        "((((((((R))))))))", "((R)((W))((R)((W)))((R)((W))((R)((W)))))",
        "((((R)(R))((R)(W)))(((W)(R))((W)(W))))"));

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
    DataTypes<Information_Model::DataType::STRING, UA_TYPES_STRING>>;

// NOLINTNEXTLINE
TYPED_TEST_SUITE(NodeBuilderTests, AllTypes);

// NOLINTNEXTLINE
TYPED_TEST(NodeBuilderTests, addDeviceNodeTypes) {
  TestFixture::testAddDeviceNode("(RW)");
}
