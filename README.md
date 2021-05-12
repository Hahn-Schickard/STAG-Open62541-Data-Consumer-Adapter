[![pipeline status](https://git.hahn-schickard.de/opc_ua_dev_group/gateway-project/data-consumer-adapter-open62541/badges/master/pipeline.svg)](https://git.hahn-schickard.de/opc_ua_dev_group/gateway-project/data-consumer-adapter-open62541/commits/master)
[![coverage report](https://git.hahn-schickard.de/opc_ua_dev_group/gateway-project/data-consumer-adapter-open62541/badges/master/coverage.svg)](https://git.hahn-schickard.de/opc_ua_dev_group/gateway-project/data-consumer-adapter-open62541/commits/master)

<img src="docs/code_documentation/vendor-logo.png" alt="" width="200"/>

# Data Consumer Adapter for Open62541 OPC UA Server Implementation

This module implements open62541 server SDK, which confirms with Data Consumer Interface

## Visual Studio Code Support

### Recomended Plugins:
* [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)  - provides linking to inellisense and code debuggers
* [C++ Intellisense](https://marketplace.visualstudio.com/items?itemName=austin.code-gnu-global) - provides code highlithing and quick navigation
* [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) - provides CMake highlithing, configruing, building
* [Clang-Format](https://marketplace.visualstudio.com/items?itemName=xaver.clang-format) - provides code formating
* [Test Explorer UI](https://marketplace.visualstudio.com/items?itemName=hbenl.vscode-test-explorer) - provides test runner integration
* [C++ TestMate](https://marketplace.visualstudio.com/items?itemName=matepek.vscode-catch2-test-adapter) - provides google-test framework adapter for Test Explorer UI

### Cmake Integration
A CMake variant file is provided with this repository for easy cmake configuration setup. This functionality requires [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) plugin to be installed. After Visual Code has been started Open Control Panel with *Cntrl + Shift + P* and type in CMake: Select Variant to start configuring your cmake project configuration. 