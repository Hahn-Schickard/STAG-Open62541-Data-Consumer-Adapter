# Changelog
## [0.3.0] - 2023.07.05 
### Added
 - `OpcuaAdapter::registrate()` method
 - `OpcuaAdapter::deregistrate()` method
 - `CallbackWrapper::ExecuteCallback` 
 - `CallbackWrapper::CallCallback` 
 - `NodeCallbackHandler::callNodeMethod()` method
 - `UA_String makeUAString(const std::string& input)` function to **Utility.hpp**
 - `UA_ByteString makeUAByteString(const std::vector<uint8_t>& input)` function to **Utility.hpp**
 - `StatusCodeNotGood` exception to **Utility.hpp**
 - `void checkStatusCode(const std::string& msg, const UA_StatusCode& status, bool uncertain_is_bad = false)` function to **Utility.hpp**
 - `void checkStatusCode(const UA_StatusCode& status, bool uncertain_is_bad = false)` function to **Utility.hpp**
 - `UA_Variant toUAVariant(const Information_Model::DataVariant& variant)` function to **Utility.hpp**
 - `Information_Model::DataVariant toDataVariant(const UA_Variant& variant)` function to **Utility.hpp**
 - write only metric support to `NodeBuilder::addWritableNode()` method
 - `NodeBuilder::addFunctionNode()` method implementation
 - data type check for `NodeCallbackHandler::writeNodeValue()` method calls

### Changed 
 - OODD dependency to fuzzy v0.2
 - `OpcuaAdapter::start()` method to accept `std::vector<Information_Model::DevicePtr>` 
 - `Historizer::registerNodeId()` method to be static
 - `Historizer::createDatabase()` method to be static
 - `NodeBuilder::add*()` methods to accept const references
 - `NodeBuilder::add*()` methods to use `checkStatusCode()` function
 - `NodeCallbackHandler::readNodeValue()` method implementation to use `toDataType()`
 - `NodeCallbackHandler::writeNodeValue()` method implementation to use `toDataVariant()`
 - **Config_Serializer** implementation to use struct field names in order to increase readability

### Removed
 - direct nlohmann_json in favor of using indirect one via HaSLL
 - direct HaSLL in favor of using indirect one via Data_Consumer_Adapter_Interface
 - direct HSCUL dependency
 - `OpcuaAdapter::handleEvent()` method
 - `FloatConversion<float, UA_TYPES_FLOAT>` test case from **NodeCallbackHandlerTests**
 - `IntegralConversion<uintmax_t, UA_StatusCode, ...>` test case from **NodeCallbackHandlerTests**

## [0.2.0] - 2023.05.12
### Added
 - fuzzy HSCUL dependency to v0.3
 - optional fuzzy OODD dependency to v0.1
 - historization option to conan recipe
 - cmake HISTORIZATION option
 - Historizer.hpp
 - Historization service
 - History View capability
 - `UA_StatusCode NodeBuilder::deleteDeviceNode()`
 - `AES128_SHA256_RSAO_AEP` enum value to SecurityPolicy struct in Config.hpp
 - `UA_ServerConfig_Discovery` struct to Config.hpp
 - `Historization` struct to Config.hpp
 - Device deregistration event handling
 - better Information_Model::Device examples

### Changed 
 - open62541 dependency to v1.3.4
 - default OPC UA Config to contain STAG project information
 - `UA_ServerConfig` into `std::unique_ptr<UA_ServerConfig>` in Configuration.hpp 
 - `Configuration` struct implementation, to use new open62541 configuration functions

### Removed
 - `std::unique_ptr<const UA_ServerConfig> Configuration::getConfig()` 
 - `Double_Use` logic error from Configuration.hpp 

### Fixed
 - fallthrough declarations in NodeCallbackHandler.cpp

## [0.1.7] - 2022.11.21
### Changed
 - conan packaging recipe
 - gtest dependency to fuzzy v1.11
 - HaSLL dependency to fuzzy v0.3
 - Variant_Visitor dependency to fuzzy v0.1
 - Data_Consumer_Adapter_Interface dependency to fuzzy v0.1

## [0.1.6] - 2022.11.03
### Added
 - OpcuaAdapter::OpcuaAdapter(ModelEventSourcePtr event_source,
     const std::string & config_filepath);
 - Configuration::Configuration(const std::string & filepath);
 - Open62541Server::Open62541Server(std::unique_ptr<Configuration>);
 - ARCHIVE Destination to CMakeLists targets
 - NodeBuilderTests
 - NodeCallbackHandlerTests
 - Open62541ServerTests

### Changed
 - Configuration::getConfig();
 - Data_Consumer_Adapter_Interface to v0.1.9
 - HaSLL to v0.3.2
 - nlohmann_json to v3.11.1
 - NodeBuilder::addDeviceNode argument from DevicePtr to NonemptyDevicePtr
 - Contribution guide
 - Code formatting rules
 - Logger deregistration to be done in the Open62541 Server dtor
 - NodeBuilder to work with NonemptyPtr implementation of Information Model
 - NodeBuilder to work with Information Model v0.2.0
 - removeLoggers() function to use HaSLL v0.3.1 deregistration mechanism

### Fixed
  - Typos in Congig.hpp API

## [0.1.5] - 2021.08.09
### Changed
 - toString(UA_Guid *input);
 - toString(const UA_String *input);
 - toString(const UA_NodeId *nodeId);
 - toString(const UA_QualifiedName *name);
 - OpcuaAdapter to no longer copy logger from DataConsumerAdapterInterface
 - Data_Consumer_Adapter_Interface to 0.1.6
 - include paths to resolve potential header name conflicts
 - main.cpp mocked Device to return proper Callbacks
 - NodeBuilder to use string::c_str() method instead of DeviceElementNodeInfo
 - wrap getMetricValue() calls in try-catch blocks

### Removed
  - DeviceElementNodeInfo

## [0.1.4] - 2020.11.16
### Added
 - Posix time conversion
 - Callback handlers for DateTime requests

### Changed
 - valgrind script to use better error detection
 - Data Consumer Adapter Interface to 0.1.4

## [0.1.3] - 2020.10.16
### Added
 - Variant_Visitor to OpcuaAdapter implementation
 - ModelEventSource to the Open62541 Adapter ctor
 - DCAI namespace to OpcuaAdapter class
 - Exception throwing for Configuration.hpp
 - cpp_compatible option for Open62541

### Changed
 - Data_Consumer_Adapter_Interface to 0.1.3
 - Event model implementation within OpcuaAdapter
 - raw C pointers into smart pointers
 - NodeManager to static NodeCallbackHandler
 - Direct C string usage to reallocation
 - Direct Variant value usage to copy usage

### Removed
 - Information_Model dependency
 - Model_Event_Handler dependency
 - Config.hpp usage
 - Config_Serializer.hpp usage
 - Logger from Configuration.hpp

## [0.1.2] - 2020.10.02
### Changed
 - Information_Model to 0.1.2
 - Model_Event_Handler to 0.1.2
 - Data_Consumer_Adapter_Interface to 0.1.2
 - OpcuaAdapter implementation to conform with Data_Consumer_Adapter_Interface
 - Variant matchers

### Added
 - Variant Visitor 0.1.0

## [0.1.1] - 2020-09.28
### Changed
 - Information_Model to 0.1.1
 - Model_Event_Handler to 0.1.1
 - Data_Consumer_Adapter_Interface to 0.1.1
 - getElementRefId() to getElementId()

## [0.1.0] - 2020.09.25
### Added
 - conan package configuration
 - License
 - Notice
 - Authors
 - Contributing
 - conan dependency handling
 - conan packaging
 - conan package integration test
 - unit_tests
 - clang formatting
