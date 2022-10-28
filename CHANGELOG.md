# Changelog
## [0.1.6] - 2022.10.28
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
 - HaSLL to v0.3.1
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
