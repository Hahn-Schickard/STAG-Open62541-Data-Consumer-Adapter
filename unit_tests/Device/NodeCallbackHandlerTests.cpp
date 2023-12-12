#include "gtest/gtest.h"

#include "NodeCallbackHandler.hpp"
#include "Utility.hpp"

using namespace open62541;

struct NodeCallbackHandlerTests : public ::testing::Test {
  std::shared_ptr<Open62541Server> server;
  CallbackWrapperPtr null_callback;

  NodeCallbackHandlerTests() : server(std::make_shared<Open62541Server>()) {
    NodeCallbackHandler::initialise(server->getServerLogger());
  }

  ~NodeCallbackHandlerTests() override { NodeCallbackHandler::destroy(); }
};

// NOLINTNEXTLINE
TEST_F(NodeCallbackHandlerTests, initialiseAndDestroy) {}

// NOLINTNEXTLINE
TEST_F(NodeCallbackHandlerTests, initialiseAndDestroyOnceMore) {}

// NOLINTNEXTLINE
TEST_F(NodeCallbackHandlerTests, addCallbackNull) {
  EXPECT_NE(UA_STATUSCODE_GOOD,
      NodeCallbackHandler::addNodeCallbacks(UA_NODEID_NULL, null_callback));
}

/*
  NodeCallbackHandlerDataConversionTests below uses a type parameter of which
  it expects a specific shape, namely:

  struct {
    using Value_Type = ...; // The type that the tests use to specify values.
    static constexpr size_t NUM_VALUES; // >= 2
    static Value_Type value(size_t i);
      // precondition: i <= NUM_VALUES
      // value(i1) and value(i2) are semantically equal iff i1=i2

    static constexpr Information_Model::DataType IM_TYPE;
      // The index into Information_Model::Data_Variant
      // In the following, we use IM_Type to refer to
      // std::variant_alternative_t<
      //   static_cast<size_t>(IM_TYPE),
      //   Information_Model::DataVariant>

    using UA_Read_Type = ...; // The type used by open62541 when reading
    static constexpr size_t UA_READ_TYPE; // The respective index into UA_TYPES
    using UA_Write_Type = ...;
      // A possibly different type used by open62541 when writing
    static constexpr size_t UA_WRITE_TYPE; // The respective index into UA_TYPES
    static void UA_Write_Type_clear(UA_Write_Type*);

    static IM_Type value2IM(Value_Type);

    // The caller is responsible for calling `UA_Write_Type_clear` on the result
    static UA_Write_Type value2Write(Value_Type);

    static bool equal(const IM_Type &, const UA_Read_Type &);
  }
*/

template <class Super, // Information_Model type, also used for reading
    class Sub, // open62541 type when writing
    Information_Model::DataType im_type_, size_t ua_super_type,
    size_t ua_sub_type>
struct SubtypeConversion {
  static constexpr size_t NUM_VALUES = 2;

  static constexpr Information_Model::DataType IM_TYPE = im_type_;

  using UA_Read_Type = Super;
  using UA_Write_Type = Sub;
  static constexpr size_t UA_READ_TYPE = ua_super_type;
  static constexpr size_t UA_WRITE_TYPE = ua_sub_type;

  static Super value2IM(Sub x) { return x; }
  static UA_Write_Type value2Write(Sub x) { return x; }
  static bool equal(const Super& v1, const UA_Read_Type& v2) {
    return v1 == v2;
  }
};

template <class Super, // Information_Model type, also used for reading
    class Sub, // open62541 type when writing
    Information_Model::DataType im_type_, size_t ua_super_type,
    size_t ua_sub_type, Sub value_1, Sub value_2>
struct IntegralConversion : public SubtypeConversion<Super, Sub, im_type_,
                                ua_super_type, ua_sub_type> {
  using Value_Type = Sub;
  static Value_Type value(size_t i) { return (i == 0 ? value_1 : value_2); }
  // NOLINTNEXTLINE(readability-identifier-naming)
  static void UA_Write_Type_clear(Sub*) {}
};

template <class Sub, size_t ua_sub_type>
struct FloatConversion
    : public SubtypeConversion<double, Sub, Information_Model::DataType::DOUBLE,
          UA_TYPES_DOUBLE, ua_sub_type> {
  using Value_Type = Sub;
  static Value_Type value(size_t i) {
    // NOLINTNEXTLINE(readability-magic-numbers)
    return (i == 0 ? 2.72 : -3.14);
  }
  // NOLINTNEXTLINE(readability-identifier-naming)
  static void UA_Write_Type_clear(Sub*) {}
};

struct StringConversion {
  using Value_Type = const char*;
  static constexpr size_t NUM_VALUES = 3;
  static Value_Type value(size_t i) {
    switch (i) {
    case 0:
      return "";
    case 1:
      return "Hello";
    case 2:
      return "world!";
    default:
      throw "internal";
    }
  }

  static constexpr Information_Model::DataType IM_TYPE =
      Information_Model::DataType::STRING;

  using UA_Read_Type = UA_String;
  using UA_Write_Type = UA_String;
  static constexpr size_t UA_READ_TYPE = UA_TYPES_STRING;
  static constexpr size_t UA_WRITE_TYPE = UA_TYPES_STRING;
  // NOLINTNEXTLINE(readability-identifier-naming)
  static void UA_Write_Type_clear(UA_Write_Type* ptr) { UA_String_clear(ptr); }

  static std::string value2IM(Value_Type x) { return x; }
  static UA_Write_Type value2Write(Value_Type s) {
    return UA_STRING_ALLOC((char*)s);
  }
  static bool equal(const std::string& v1, const UA_Read_Type& v2) {
    return (v1.length() == v2.length) &&
        (0 == memcmp(v1.data(), v2.data, v2.length));
  }
};

struct TimeConversion {
  using Value_Type = std::time_t;
  static constexpr size_t NUM_VALUES = 2;
  static Value_Type value(size_t i) {
    return std::chrono::system_clock::to_time_t(i == 0
            ? std::chrono::time_point<std::chrono::system_clock>::min()
            : std::chrono::time_point<std::chrono::system_clock>::max());
  }

  static constexpr Information_Model::DataType IM_TYPE =
      Information_Model::DataType::TIME;

  using UA_Read_Type = UA_DateTime;
  using UA_Write_Type = UA_DateTime;
  static constexpr size_t UA_READ_TYPE = UA_TYPES_DATETIME;
  static constexpr size_t UA_WRITE_TYPE = UA_TYPES_DATETIME;
  // NOLINTNEXTLINE(readability-identifier-naming)
  static void UA_Write_Type_clear(UA_Write_Type* ptr) {
    UA_DateTime_clear(ptr);
  }

  static Information_Model::DateTime value2IM(Value_Type t) { return t; }
  static UA_Write_Type value2Write(Value_Type t) {
    return UA_DateTime_fromUnixTime(t);
  }
  static bool equal(
      const Information_Model::DateTime& v1, const UA_Read_Type& v2) {
    return v1.getValue() == UA_DateTime_toUnixTime(v2);
  }
};

struct ByteStringConversion {
  using Value_Type = std::string;
  static constexpr size_t NUM_VALUES = 4;
  static Value_Type value(size_t i) {
    switch (i) {
    case 0:
      return std::string();
    case 1:
      /**
       * This string is used as a c string buffer in tests, thus we allocate
       * more memory as used by the std::string ctor
       */
      // NOLINTNEXTLINE(bugprone-string-constructor)
      return std::string("", 1);
    case 2:
      return std::string("\000\377", 2);
    case 3:
      return std::string("\377", 1);
    default:
      throw "internal";
    }
  }

  static constexpr Information_Model::DataType IM_TYPE =
      Information_Model::DataType::OPAQUE;

  using UA_Read_Type = UA_ByteString;
  using UA_Write_Type = UA_ByteString;
  static constexpr size_t UA_READ_TYPE = UA_TYPES_BYTESTRING;
  static constexpr size_t UA_WRITE_TYPE = UA_TYPES_BYTESTRING;
  // NOLINTNEXTLINE(readability-identifier-naming)
  static void UA_Write_Type_clear(UA_Write_Type* ptr) {
    UA_ByteString_clear(ptr);
  }

  static std::vector<uint8_t> value2IM(const Value_Type& s) {
    std::vector<uint8_t> ret;
    for (auto c : s) {
      ret.push_back(c);
    }
    return ret;
  }

  static UA_Write_Type value2Write(const Value_Type& s) {
    UA_ByteString ret;
    EXPECT_EQ(UA_STATUSCODE_GOOD, UA_ByteString_allocBuffer(&ret, s.length()));
    memcpy(ret.data, s.data(), s.length());
    return ret;
  }

  static bool equal(const std::vector<uint8_t>& v1, const UA_Read_Type& v2) {
    return (v1.size() == v2.length) &&
        (0 == memcmp(v1.data(), v2.data, v2.length));
  }
};

using AllConversions = ::testing::Types<
    IntegralConversion<bool, bool, Information_Model::DataType::BOOLEAN,
        UA_TYPES_BOOLEAN, UA_TYPES_BOOLEAN, true, false>,
    IntegralConversion<intmax_t, int8_t, Information_Model::DataType::INTEGER,
        // NOLINTNEXTLINE(readability-magic-numbers)
        UA_TYPES_intmax, UA_TYPES_SBYTE, 42, -1>,
    IntegralConversion<uintmax_t, uint8_t,
        Information_Model::DataType::UNSIGNED_INTEGER, UA_TYPES_uintmax,
        UA_TYPES_BYTE, 13, 91>, // NOLINT(readability-magic-numbers)
    IntegralConversion<intmax_t, int16_t, Information_Model::DataType::INTEGER,
        // NOLINTNEXTLINE(readability-magic-numbers)
        UA_TYPES_intmax, UA_TYPES_INT16, 42, -1>,
    IntegralConversion<uintmax_t, uint16_t,
        Information_Model::DataType::UNSIGNED_INTEGER, UA_TYPES_uintmax,
        UA_TYPES_UINT16, 13, 91>, // NOLINT(readability-magic-numbers)
    IntegralConversion<intmax_t, int32_t, Information_Model::DataType::INTEGER,
        // NOLINTNEXTLINE(readability-magic-numbers)
        UA_TYPES_intmax, UA_TYPES_INT32, 42, -1>,
    IntegralConversion<uintmax_t, uint32_t,
        Information_Model::DataType::UNSIGNED_INTEGER, UA_TYPES_uintmax,
        UA_TYPES_UINT32, 13, 91>, // NOLINT(readability-magic-numbers)
    IntegralConversion<intmax_t, int64_t, Information_Model::DataType::INTEGER,
        // NOLINTNEXTLINE(readability-magic-numbers)
        UA_TYPES_intmax, UA_TYPES_INT64, 42, -1>,
    IntegralConversion<uintmax_t, uint64_t,
        Information_Model::DataType::UNSIGNED_INTEGER, UA_TYPES_uintmax,
        UA_TYPES_UINT64, 13, 91>, // NOLINT(readability-magic-numbers)
    FloatConversion<double, UA_TYPES_DOUBLE>, StringConversion, TimeConversion,
    ByteStringConversion>;

template <class Type_>
struct NodeCallbackHandlerDataConversionTests
    : public NodeCallbackHandlerTests {
  using Type = Type_;

  using IM_Type = std::variant_alternative_t<static_cast<size_t>(Type::IM_TYPE),
      Information_Model::DataVariant>;

  UA_NodeId node1, node2;

  // the (single) resource that all callbacks use
  Information_Model::DataVariant im_value;

  std::shared_ptr<CallbackWrapper> read_only_callbacks =
      std::make_shared<CallbackWrapper>(Type::IM_TYPE,
          [this]() -> Information_Model::DataVariant { return im_value; });

  std::shared_ptr<CallbackWrapper> read_write_callbacks =
      std::make_shared<CallbackWrapper>(
          Type::IM_TYPE,
          [this]() -> Information_Model::DataVariant { return im_value; },
          [this](const Information_Model::DataVariant& value) {
            im_value = value;
          });

  NodeCallbackHandlerDataConversionTests() {
    node1 = UA_NODEID_NULL;
    EXPECT_EQ(
        UA_STATUSCODE_GOOD, UA_NodeId_parse(&node2, UA_STRING((char*)"i=13")));
  }

private:
  Information_Model::DataVariant initImValue(size_t value_index) {
    return Information_Model::DataVariant(
        std::in_place_index<static_cast<size_t>(Type::IM_TYPE)>,
        Type::value2IM(Type::value(value_index)));
  }

  // The caller is responsible for calling `UA_DataValue_clear` on `dst`
  void initUaValue(UA_DataValue& dst, size_t value_index) {
    auto ua_val = Type::value2Write(Type::value(value_index));
    dst.hasValue = true;
    dst.hasStatus = false;
    dst.hasSourceTimestamp = false;
    dst.hasServerTimestamp = false;
    dst.hasSourcePicoseconds = false;
    dst.hasServerPicoseconds = false;
    EXPECT_EQ(UA_STATUSCODE_GOOD,
        UA_Variant_setScalarCopy(
            &dst.value, &ua_val, &UA_TYPES[Type::UA_WRITE_TYPE]));
    Type::UA_Write_Type_clear(&ua_val);
  }

  // The caller is responsible for calling `UA_DataValue_clear` on `ua_value`
  UA_StatusCode invokeRead(const UA_NodeId& node, UA_DataValue& ua_value) {
    return NodeCallbackHandler::readNodeValue(nullptr, nullptr, nullptr, &node,
        nullptr, UA_FALSE, nullptr, &ua_value);
  }

  UA_StatusCode invokeWrite(
      const UA_NodeId& node, const UA_DataValue& ua_value) {
    return NodeCallbackHandler::writeNodeValue(
        nullptr, nullptr, nullptr, &node, nullptr, nullptr, &ua_value);
  }

  void checkNoReadCallback(const UA_NodeId& node) {
    UA_DataValue ua_value;
    EXPECT_NE(UA_STATUSCODE_GOOD, invokeRead(node, ua_value));
  }

  void checkNoWriteCallback(const UA_NodeId& node) {
    UA_DataValue ua_value;
    initUaValue(ua_value, 0);
    EXPECT_NE(UA_STATUSCODE_GOOD, invokeWrite(node, ua_value));
    UA_DataValue_clear(&ua_value);
  }

  void checkRead(const UA_NodeId& node, size_t nominal_value) {
    UA_DataValue ua_value;
    EXPECT_EQ(UA_STATUSCODE_GOOD, invokeRead(node, ua_value));
    EXPECT_TRUE(ua_value.hasValue);
    EXPECT_TRUE(UA_Variant_hasScalarType(
        &ua_value.value, &UA_TYPES[Type::UA_READ_TYPE]));
    for (size_t i = 0; i < Type::NUM_VALUES; ++i) {
      EXPECT_EQ(i == nominal_value,
          Type::equal(Type::value2IM(Type::value(i)),
              *((typename Type::UA_Read_Type*)ua_value.value.data)))
          << i << "," << nominal_value;
    }
    UA_DataValue_clear(&ua_value);
  }

public:
  void testNoCallbacks(const UA_NodeId& node) {
    checkNoReadCallback(node);
    checkNoWriteCallback(node);
  }

  void testReadOnlyCallbacks(const UA_NodeId& node) {
    for (size_t value = 0; value < Type::NUM_VALUES; ++value) {
      im_value = initImValue(value);
      checkRead(node, value);
    }
    checkNoWriteCallback(node);
  }

  void testReadWriteCallbacks(const UA_NodeId& node) {
    UA_DataValue ua_v;

    for (size_t value = 0; value < Type::NUM_VALUES; ++value) {
      initUaValue(ua_v, value);
      EXPECT_EQ(UA_STATUSCODE_GOOD, invokeWrite(node, ua_v));

      EXPECT_EQ(im_value, initImValue(value));
      checkRead(node, value);
      UA_DataValue_clear(&ua_v);
    }
  }
};

// NOLINTNEXTLINE
TYPED_TEST_SUITE(NodeCallbackHandlerDataConversionTests, AllConversions);

// NOLINTNEXTLINE
TYPED_TEST(NodeCallbackHandlerDataConversionTests, addNoCallback) {
  EXPECT_EQ(UA_STATUSCODE_GOOD,
      NodeCallbackHandler::addNodeCallbacks(
          TestFixture::node1, std::make_shared<CallbackWrapper>()));
  TestFixture::testNoCallbacks(TestFixture::node1);
  TestFixture::testNoCallbacks(TestFixture::node2);
}

// NOLINTNEXTLINE
TYPED_TEST(NodeCallbackHandlerDataConversionTests, addReadCallback) {
  EXPECT_EQ(UA_STATUSCODE_GOOD,
      NodeCallbackHandler::addNodeCallbacks(
          TestFixture::node1, TestFixture::read_only_callbacks));
  TestFixture::testReadOnlyCallbacks(TestFixture::node1);
  TestFixture::testNoCallbacks(TestFixture::node2);
}

// NOLINTNEXTLINE
TYPED_TEST(NodeCallbackHandlerDataConversionTests, addRWCallbacks) {
  EXPECT_EQ(UA_STATUSCODE_GOOD,
      NodeCallbackHandler::addNodeCallbacks(
          TestFixture::node1, TestFixture::read_write_callbacks));
  TestFixture::testReadWriteCallbacks(TestFixture::node1);
  TestFixture::testNoCallbacks(TestFixture::node2);
}

// NOLINTNEXTLINE
TYPED_TEST(NodeCallbackHandlerDataConversionTests, addTwoCallbacks) {
  EXPECT_EQ(UA_STATUSCODE_GOOD,
      NodeCallbackHandler::addNodeCallbacks(
          TestFixture::node1, TestFixture::read_only_callbacks));
  EXPECT_EQ(UA_STATUSCODE_GOOD,
      NodeCallbackHandler::addNodeCallbacks(
          TestFixture::node2, TestFixture::read_write_callbacks));

  TestFixture::testReadOnlyCallbacks(TestFixture::node1);
  TestFixture::testReadWriteCallbacks(TestFixture::node2);
}

// NOLINTNEXTLINE
TYPED_TEST(NodeCallbackHandlerDataConversionTests, addCallbacksTwice) {
  EXPECT_EQ(UA_STATUSCODE_GOOD,
      NodeCallbackHandler::addNodeCallbacks(
          TestFixture::node1, TestFixture::read_only_callbacks));
  EXPECT_NE(UA_STATUSCODE_GOOD,
      NodeCallbackHandler::addNodeCallbacks(
          TestFixture::node1, TestFixture::read_write_callbacks));

  TestFixture::testReadOnlyCallbacks(TestFixture::node1);
}

// NOLINTNEXTLINE
TYPED_TEST(NodeCallbackHandlerDataConversionTests, removeCallbacks) {
  EXPECT_EQ(UA_STATUSCODE_GOOD,
      NodeCallbackHandler::addNodeCallbacks(
          TestFixture::node1, TestFixture::read_only_callbacks));
  EXPECT_EQ(UA_STATUSCODE_GOOD,
      NodeCallbackHandler::addNodeCallbacks(
          TestFixture::node2, TestFixture::read_write_callbacks));

  // so far the same situation as in addTwoCallbacks

  UA_NodeId node = TestFixture::node1;
  EXPECT_EQ(
      UA_STATUSCODE_GOOD, NodeCallbackHandler::removeNodeCallbacks(&node));

  TestFixture::testNoCallbacks(TestFixture::node1);
  TestFixture::testReadWriteCallbacks(TestFixture::node2);
}

// NOLINTNEXTLINE
TYPED_TEST(NodeCallbackHandlerDataConversionTests, removeNonexistingCallbacks) {
  EXPECT_EQ(UA_STATUSCODE_GOOD,
      NodeCallbackHandler::addNodeCallbacks(
          TestFixture::node1, TestFixture::read_only_callbacks));

  UA_NodeId node = TestFixture::node2;
  EXPECT_NE(
      UA_STATUSCODE_GOOD, NodeCallbackHandler::removeNodeCallbacks(&node));
}
