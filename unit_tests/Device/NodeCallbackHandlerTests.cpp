#include "gtest/gtest.h"

#include "NodeCallbackHandler.hpp"
#include "Utility.hpp"

namespace NodeCallbackHandlerTests {

using namespace open62541;

struct NodeCallbackHandlerTests : public ::testing::Test {
  std::shared_ptr<Open62541Server> server;
  CallbackWrapperPtr null_callback;

  NodeCallbackHandlerTests()
    : server(std::make_shared<Open62541Server>())
  {
    NodeCallbackHandler::initialise(server->getServerLogger());
  }

  ~NodeCallbackHandlerTests() {
    NodeCallbackHandler::destroy();
  }
};

TEST_F(NodeCallbackHandlerTests, initialiseAndDestroy) {}
TEST_F(NodeCallbackHandlerTests, initialiseAndDestroyOnceMore) {}

TEST_F(NodeCallbackHandlerTests, addCallbackNull) {
  EXPECT_NE(UA_STATUSCODE_GOOD,
    NodeCallbackHandler::addNodeCallbacks(UA_NODEID_NULL, null_callback));
}

/*
  NodeCallbackHandlerDataConversionTests below uses a type parameter of which
  it expects a specific shape, namely:

  struct {
    using Value_Type = ...; // The type that the tests use to specify values.
    static constexpr size_t num_values; // >= 2
    static Value_Type value(size_t i);
      // precondition: i <= num_values
      // value(i1) and value(i2) are semantically equal iff i1=i2

    static constexpr Information_Model::DataType im_type;
      // The index into Information_Model::Data_Variant
      // In the following, we use IM_Type to refer to
      // std::variant_alternative_t<
      //   static_cast<size_t>(im_type),
      //   Information_Model::DataVariant>

    using UA_Read_Type = ...; // The type used by open62541 when reading
    static constexpr size_t ua_read_type; // The respective index into UA_TYPES
    using UA_Write_Type = ...;
      // A possibly different type used by open62541 when writing
    static constexpr size_t ua_write_type; // The respective index into UA_TYPES

    static IM_Type Value2IM(Value_Type);
    static UA_Write_Type Value2Write(Value_Type);
    static bool equal(const IM_Type &, const UA_Read_Type &);
  }
*/

template <
  class Super, // Information_Model type, also used for reading
  class Sub, // open62541 type when writing
  Information_Model::DataType im_type_,
  size_t ua_super_type,
  size_t ua_sub_type>
struct SubtypeConversion {
  static constexpr size_t num_values = 2;

  static constexpr Information_Model::DataType im_type = im_type_;

  using UA_Read_Type = Super;
  using UA_Write_Type = Sub;
  static constexpr size_t ua_read_type = ua_super_type;
  static constexpr size_t ua_write_type = ua_sub_type;

  static Super Value2IM(Sub x) { return x; }
  static UA_Write_Type Value2Write(Sub x) { return x; }
  static bool equal(const Super & v1, const UA_Read_Type & v2) {
    return v1 == v2;
  }
};

template <
  class Super, // Information_Model type, also used for reading
  class Sub, // open62541 type when writing
  Information_Model::DataType im_type_,
  size_t ua_super_type,
  size_t ua_sub_type,
  Sub value_1,
  Sub value_2>
struct IntegralConversion
  : public SubtypeConversion<Super,Sub,im_type_,ua_super_type,ua_sub_type>
{
  using Value_Type = Sub;
  static Value_Type value(size_t i) { return (i == 0 ? value_1 : value_2); }
};

template <
  class Sub,
  size_t ua_sub_type>
struct FloatConversion
  : public SubtypeConversion<double,Sub,
    Information_Model::DataType::DOUBLE, UA_TYPES_DOUBLE, ua_sub_type>
{
  using Value_Type = Sub;
  static Value_Type value(size_t i) { return (i == 0 ? 2.72 : -3.14); }
};

struct StringConversion {
  using Value_Type = const char *;
  static constexpr size_t num_values = 3;
  static Value_Type value(size_t i) {
    switch (i) {
      case 0: return "";
      case 1: return "Hello";
      case 2: return "world!";
      default: throw "internal";
    }
  }

  static constexpr Information_Model::DataType im_type
    = Information_Model::DataType::STRING;

  using UA_Read_Type = UA_String;
  using UA_Write_Type = UA_String;
  static constexpr size_t ua_read_type = UA_TYPES_STRING;
  static constexpr size_t ua_write_type = UA_TYPES_STRING;

  static std::string Value2IM(Value_Type x) { return x; }
  static UA_Write_Type Value2Write(Value_Type s) {
    return UA_STRING((char *)s);
  }
  static bool equal(const std::string & v1, const UA_Read_Type & v2) {
    return (v1.length() == v2.length)
      && (0 == memcmp(v1.data(), v2.data, v2.length));
  }
};

struct TimeConversion {
  using Value_Type = std::time_t;
  static constexpr size_t num_values = 2;
  static Value_Type value(size_t i) {
    return std::chrono::system_clock::to_time_t(
      i == 0
      ? std::chrono::time_point<std::chrono::system_clock>::min()
      : std::chrono::time_point<std::chrono::system_clock>::max());
  }

  static constexpr Information_Model::DataType im_type
    = Information_Model::DataType::TIME;

  using UA_Read_Type = UA_DateTime;
  using UA_Write_Type = UA_DateTime;
  static constexpr size_t ua_read_type = UA_TYPES_DATETIME;
  static constexpr size_t ua_write_type = UA_TYPES_DATETIME;

  static Information_Model::DateTime Value2IM(Value_Type t) { return t; }
  static UA_Write_Type Value2Write(Value_Type t) {
    return UA_DateTime_fromUnixTime(t);
  }
  static bool equal(
    const Information_Model::DateTime & v1, const UA_Read_Type & v2)
  {
    return v1.getValue() == UA_DateTime_toUnixTime(v2);
  }
};

struct ByteStringConversion {
  using Value_Type = std::string;
  static constexpr size_t num_values = 4;
  static Value_Type value(size_t i) {
    switch (i) {
      case 0: return std::string();
      case 1: return std::string("", 1);
      case 2: return std::string("\000\377", 2);
      case 3: return std::string("\377", 1);
      default: throw "internal";
    }
  }

  static constexpr Information_Model::DataType im_type
    = Information_Model::DataType::OPAQUE;

  using UA_Read_Type = UA_ByteString;
  using UA_Write_Type = UA_ByteString;
  static constexpr size_t ua_read_type = UA_TYPES_BYTESTRING;
  static constexpr size_t ua_write_type = UA_TYPES_BYTESTRING;

  static std::vector<uint8_t> Value2IM(Value_Type s) {
    std::vector<uint8_t> ret;
    for (auto c:s)
      ret.push_back(c);
    return ret;
  }
  static UA_Write_Type Value2Write(Value_Type s) {
    UA_ByteString ret;
    EXPECT_EQ(UA_STATUSCODE_GOOD, UA_ByteString_allocBuffer(&ret, s.length()));
    memcpy(ret.data, s.data(), s.length());
    return ret;
  }
  static bool equal(const std::vector<uint8_t> & v1, const UA_Read_Type & v2) {
    return (v1.size() == v2.length)
      && (0 == memcmp(v1.data(), v2.data, v2.length));
  }
};

using AllConversions = ::testing::Types<
  IntegralConversion<bool, bool,
    Information_Model::DataType::BOOLEAN, UA_TYPES_BOOLEAN, UA_TYPES_BOOLEAN,
    true, false>,
  IntegralConversion<int64_t, int8_t,
    Information_Model::DataType::INTEGER, UA_TYPES_INT64, UA_TYPES_SBYTE,
    42, -1>,
  IntegralConversion<uint64_t, uint8_t,
    Information_Model::DataType::UNSIGNED_INTEGER,
    UA_TYPES_UINT64, UA_TYPES_BYTE,
    13, 91>,
  IntegralConversion<int64_t, int16_t,
    Information_Model::DataType::INTEGER, UA_TYPES_INT64, UA_TYPES_INT16,
    42, -1>,
  IntegralConversion<uint64_t, uint16_t,
    Information_Model::DataType::UNSIGNED_INTEGER,
    UA_TYPES_UINT64, UA_TYPES_UINT16,
    13, 91>,
  IntegralConversion<int64_t, int32_t,
    Information_Model::DataType::INTEGER, UA_TYPES_INT64, UA_TYPES_INT32,
    42, -1>,
  IntegralConversion<uint64_t, uint32_t,
    Information_Model::DataType::UNSIGNED_INTEGER,
    UA_TYPES_UINT64, UA_TYPES_UINT32,
    13, 91>,
  IntegralConversion<int64_t, int64_t,
    Information_Model::DataType::INTEGER, UA_TYPES_INT64, UA_TYPES_INT64,
    42, -1>,
  IntegralConversion<uint64_t, uint64_t,
    Information_Model::DataType::UNSIGNED_INTEGER,
    UA_TYPES_UINT64, UA_TYPES_UINT64,
    13, 91>,
  FloatConversion<float, UA_TYPES_FLOAT>,
  FloatConversion<double, UA_TYPES_DOUBLE>,
  IntegralConversion<uint64_t, UA_StatusCode,
    Information_Model::DataType::UNSIGNED_INTEGER,
    UA_TYPES_UINT64, UA_TYPES_STATUSCODE,
    UA_STATUSCODE_GOOD, UA_STATUSCODE_BADOUTOFMEMORY>,
  StringConversion,
  TimeConversion,
  ByteStringConversion
  >;

template <class Type_>
struct NodeCallbackHandlerDataConversionTests : public NodeCallbackHandlerTests
{
  using Type = Type_;

  using IM_Type = std::variant_alternative_t<
    static_cast<size_t>(Type::im_type),
    Information_Model::DataVariant>;

  UA_NodeId node1,node2;

  Information_Model::DataVariant im_value;
    // the (single) resource that all callbacks use

  std::shared_ptr<CallbackWrapper> read_only_callbacks =
    std::make_shared<CallbackWrapper>(
      Type::im_type,
      [this]()->Information_Model::DataVariant { return im_value; });

  std::shared_ptr<CallbackWrapper> read_write_callbacks =
    std::make_shared<CallbackWrapper>(
      Type::im_type,
      [this]()->Information_Model::DataVariant { return im_value; },
      [this](Information_Model::DataVariant value){ im_value = value; });

  NodeCallbackHandlerDataConversionTests() {
    node1 = UA_NODEID_NULL;
    EXPECT_EQ(UA_STATUSCODE_GOOD,
      UA_NodeId_parse(&node2, UA_STRING((char *) "i=13")));
  }

private:
  Information_Model::DataVariant init_im_value(size_t value_index) {
    return Information_Model::DataVariant(
      std::in_place_index<static_cast<size_t>(Type::im_type)>,
      Type::Value2IM(Type::value(value_index)));
  }

  void init_ua_value(UA_DataValue & dst, size_t value_index) {
    auto ua_val = Type::Value2Write(Type::value(value_index));
    dst.hasValue = true;
    dst.hasStatus = false;
    dst.hasSourceTimestamp = false;
    dst.hasServerTimestamp = false;
    dst.hasSourcePicoseconds = false;
    dst.hasServerPicoseconds = false;
    EXPECT_EQ(UA_STATUSCODE_GOOD, UA_Variant_setScalarCopy(
      &dst.value, &ua_val, &UA_TYPES[Type::ua_write_type]));
  }

  UA_StatusCode invoke_read(const UA_NodeId & node, UA_DataValue & ua_value) {
    return NodeCallbackHandler::readNodeValue(
      nullptr, nullptr, nullptr,
      &node,
      nullptr, UA_FALSE, nullptr,
      &ua_value);
  }

  UA_StatusCode invoke_write(
    const UA_NodeId & node, const UA_DataValue & ua_value)
  {
    return NodeCallbackHandler::writeNodeValue(
      nullptr, nullptr, nullptr,
      &node,
      nullptr, nullptr,
      &ua_value);
  }

  void check_no_read_callback(const UA_NodeId & node) {
    UA_DataValue ua_value;
    EXPECT_NE(UA_STATUSCODE_GOOD, invoke_read(node,ua_value));
  }

  void check_no_write_callback(const UA_NodeId & node) {
    UA_DataValue ua_value;
    init_ua_value(ua_value, 0);
    EXPECT_NE(UA_STATUSCODE_GOOD, invoke_write(node, ua_value));
  }

  void check_read(const UA_NodeId & node, size_t nominal_value) {
    UA_DataValue ua_value;
    EXPECT_EQ(UA_STATUSCODE_GOOD, invoke_read(node,ua_value));
    EXPECT_TRUE(ua_value.hasValue);
    EXPECT_TRUE(UA_Variant_hasScalarType(
      &ua_value.value, &UA_TYPES[Type::ua_read_type]));
    for (size_t i=0; i<Type::num_values; ++i)
      EXPECT_EQ(
        i == nominal_value,
        Type::equal(
          Type::Value2IM(Type::value(i)),
          *((typename Type::UA_Read_Type *) ua_value.value.data))
        ) << i << "," << nominal_value;
  }

public:
  void test_no_callbacks(const UA_NodeId & node) {
    check_no_read_callback(node);
    check_no_write_callback(node);
  }

  void test_read_only_callbacks(const UA_NodeId & node) {
    for (size_t value=0; value<Type::num_values; ++value) {
      im_value = init_im_value(value);
      check_read(node, value);
    }
    check_no_write_callback(node);
  }

  void test_read_write_callbacks(const UA_NodeId & node) {
    UA_DataValue ua_v;

    for (size_t value=0; value<Type::num_values; ++value) {
      init_ua_value(ua_v, value);
      EXPECT_EQ(UA_STATUSCODE_GOOD, invoke_write(node,ua_v));

      EXPECT_EQ(im_value, init_im_value(value));
      check_read(node, value);
    }
  }
};
TYPED_TEST_SUITE(NodeCallbackHandlerDataConversionTests, AllConversions);

TYPED_TEST(NodeCallbackHandlerDataConversionTests, addNoCallback) {
  EXPECT_EQ(UA_STATUSCODE_GOOD,
    NodeCallbackHandler::addNodeCallbacks(
      TestFixture::node1, std::make_shared<CallbackWrapper>()));
  TestFixture::test_no_callbacks(TestFixture::node1);
  TestFixture::test_no_callbacks(TestFixture::node2);
}

TYPED_TEST(NodeCallbackHandlerDataConversionTests, addReadCallback) {
  EXPECT_EQ(UA_STATUSCODE_GOOD,
    NodeCallbackHandler::addNodeCallbacks(
      TestFixture::node1, TestFixture::read_only_callbacks));
  TestFixture::test_read_only_callbacks(TestFixture::node1);
  TestFixture::test_no_callbacks(TestFixture::node2);
}

TYPED_TEST(NodeCallbackHandlerDataConversionTests, addRWCallbacks) {
  EXPECT_EQ(UA_STATUSCODE_GOOD,
    NodeCallbackHandler::addNodeCallbacks(
      TestFixture::node1, TestFixture::read_write_callbacks));
  TestFixture::test_read_write_callbacks(TestFixture::node1);
  TestFixture::test_no_callbacks(TestFixture::node2);
}

TYPED_TEST(NodeCallbackHandlerDataConversionTests, addTwoCallbacks) {
  EXPECT_EQ(UA_STATUSCODE_GOOD,
    NodeCallbackHandler::addNodeCallbacks(
      TestFixture::node1, TestFixture::read_only_callbacks));
  EXPECT_EQ(UA_STATUSCODE_GOOD,
    NodeCallbackHandler::addNodeCallbacks(
      TestFixture::node2, TestFixture::read_write_callbacks));

  TestFixture::test_read_only_callbacks(TestFixture::node1);
  TestFixture::test_read_write_callbacks(TestFixture::node2);
}

TYPED_TEST(NodeCallbackHandlerDataConversionTests, addCallbacksTwice) {
  EXPECT_EQ(UA_STATUSCODE_GOOD,
    NodeCallbackHandler::addNodeCallbacks(
      TestFixture::node1, TestFixture::read_only_callbacks));
  EXPECT_NE(UA_STATUSCODE_GOOD,
    NodeCallbackHandler::addNodeCallbacks(
      TestFixture::node1, TestFixture::read_write_callbacks));

  TestFixture::test_read_only_callbacks(TestFixture::node1);
}

TYPED_TEST(NodeCallbackHandlerDataConversionTests, removeCallbacks) {
  EXPECT_EQ(UA_STATUSCODE_GOOD,
    NodeCallbackHandler::addNodeCallbacks(
      TestFixture::node1, TestFixture::read_only_callbacks));
  EXPECT_EQ(UA_STATUSCODE_GOOD,
    NodeCallbackHandler::addNodeCallbacks(
      TestFixture::node2, TestFixture::read_write_callbacks));

  // so far the same situation as in addTwoCallbacks

  UA_NodeId node = TestFixture::node1;
  EXPECT_EQ(UA_STATUSCODE_GOOD,
    NodeCallbackHandler::removeNodeCallbacks(&node));

  TestFixture::test_no_callbacks(TestFixture::node1);
  TestFixture::test_read_write_callbacks(TestFixture::node2);
}

TYPED_TEST(NodeCallbackHandlerDataConversionTests, removeNonexistingCallbacks) {
  EXPECT_EQ(UA_STATUSCODE_GOOD,
    NodeCallbackHandler::addNodeCallbacks(
      TestFixture::node1, TestFixture::read_only_callbacks));

  UA_NodeId node = TestFixture::node2;
  EXPECT_NE(UA_STATUSCODE_GOOD,
    NodeCallbackHandler::removeNodeCallbacks(&node));
}

} // namespace
