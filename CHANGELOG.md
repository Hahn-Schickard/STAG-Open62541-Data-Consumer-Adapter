# Changelog
## [0.4.0] - 2025.03.11
### Added
 - Windows support

### Changed
 - `Open62541` to v1.3.15
 - `Open62541` to be linked as a shared library
 - `Data_Consumer_Adapter_Interface` to v0.3
 - `OODD` to v0.3
 - `GTest` to v1.16
 - `GTest` to be linked as a shared library
 - `Historization` service to default on 
 - `OpcuaAdapter::start()` method to use `const Devices&` argument
 - `OpcuaAdapter::OpcuaAdapter(ModelEventSourcePtr)` to `OpcuaAdapter::OpcuaAdapter(const ModelEventSourcePtr&)`
 - `NonemptyPointer` to `Nonempty`
 - `Information_Model::DataType` enum values to Camel_Case
 - `Information_Model::ElementType` enum values to Camel_Case
 - `OODD::DataType` enum values to Camel_Case
 - `OODD::ColumnConstraint` enum values to Camel_Case
 - `OODD::FilterType` enum values to Camel_Case
 - `DataLocation`enum values to CamelCase
 - `Open62541_Logger` enum values to CamelCase
 
## [0.3.7] - 2025.01.15
### Added 
 - Method mocks to example runner 

### Fixed 
 - `NodeBuilder::addFunctionNode()` not calling `UA_Server_addMethodNode` due to a missing {
 - Incorrect function result condition check in `NodeBuilder::addFunctionNode()` 
 - `NodeCallbackHandler::callNodeMethod()` not setting result status to `UA_STATUSCODE_GOOD` upon successful callback call

### Changed
 - `NodeBuilder::addFunctionNode()` to use lambdas, instead of `std::bind()` when creating `CallbackWrapperPtr` instances

## [0.3.6] - 2024.08.13
### Changed
 - Cmake Historization option to default to OFF
 - Conan to default to historization:False

## [0.3.5] - 2024.01.30
### Added
 - `NodeId` class to control `UA_NodeId` lifetime in `NodeCallbackHandler`
 - `~Config` to clear `access_credentials` fields
 - `std::string toString(const UA_ExpandedNodeId& id)` function to utilities
 - logging for each deletion stage in `NodeBuilder::deleteDeviceNode()`
 - **try-catch** block for `NodeCalbackMap::AlreadyErased` exception in 
   `NodeCallbackHandler::removeNodeCallbacks()` method
 - `UA_MdnsDiscoveryConfiguration` serialization in `UA_ServerConfig_Discovery`

### Fixed
 - memory leak in `NodeCallbackHandler::node_calbacks_map_` where the destructor 
   of the open62541 data type `UA_NodeId` did not clear the data allocated for 
   the `UA_NodeId`
 - memory leaks in `Configuration::Configuration(const string& filepath)` where
   the fields initialized by `Configuration::Configuration(bool)` ctor were not 
   cleared.
 - memory leaks where temporary objects of type `UA_BrowseResult`,
   `UA_DataValue`, `UA_LocalizedText`, `UA_NodeId`, `UA_QualifiedName`,
   `UA_String`, `UA_VariableAttributes`, and `UA_Variant` were not cleared

### Changed 
  - `Open62541Server::runnable()` to continually retry restarting the server
    while the adapter is running
  - `NodeCallbackHandler::node_calbacks_map_` to use `NodeId` instead of `UA_NodeId`

## [0.3.4] - 2023.12.06
### Changed
 - logging function calls in `OpcuaAdapter` to avoid using `SeverityLevel` enum
 - logging function calls in `Open62541Server` to avoid using `SeverityLevel` enum
 - logging function calls in `NodeBuilder` to avoid using `SeverityLevel` enum
 - logging function calls in main.cpp to avoid using `SeverityLevel` enum
 - `Open62541Server::stop()` method to return **true** if open62541 server is not running
 - `OpcuaAdapter::stop()` method to first check if open62541 server is running before stopping it

### Fixed
 - `Open62541Server::runnable()` critical log message formatting
 - `Open62541Server::start()` critical log message formatting

## [0.3.3] - 2023.12.01
### Changed 
 - `NodeCallbackHandler::node_calbacks_map_` to `Threadsafe::UnorderedMap<UA_NodeId, CallbackWrapperPtr>`

## [0.3.2] - 2023.10.13
### Added 
 - `NodeBuilder::removeDataSources()` method 

### Fixed
 - NodeBuilder not removing data source callbacks on deregistration event

### Changed 
 - HaSLL logger registration/deregistration to be handled by `OpcuaAdapter`
 - `NodeCallbackHandler::add/removeCallback()` methods to use info logger instead of trace
 - Logger names to not have any whitespaces
 - `NodeCallbackHandler::readNodeValue()` method to remove nodes when callback throws an unhandled exception
 - `NodeCallbackHandler::writeNodeValue()` method to remove nodes when callback throws an unhandled exception
 - `NodeCallbackHandler::callNodeMethod()` method to remove nodes when callback throws an unhandled exception

## [0.3.1] - 2023.09.22
### Added
 - `COVERAGE_TRACKING` cmake option 
 - `CMAKE_CXX_STANDARD` variable
 - plantuml support to doxygen
 - `CMAKE_EXE_LINKER_FLAGS` to use old dynamic linker tags

### Changed
 - conan recipe to use conan v2 syntax
 - conan cmake integration to use conan v2 engine
 - CMake requirement to 3.24
 - CONTRIBUTING.md
 - doxygen documentation style
 - README.md
 - `const Config deserializeConfig(const std::string& file_path)` function to no longer return `const Config`
 - `historization` field to be optional in `open62541::Config` struct

### Removed
 - CMake package configuration
 - CMake `FIND_INSTALLED_DEPENDENCIES` macro
 - CMake `EXPORT` targets

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
