# Introduction
## C++17 Lightweight Machine to Machine (LwM2M) Server Implementation
### Brief description

This is a module for C++17 based LwM2M Server implementation. It uses [ASIO](https://think-async.com/Asio/) to manage the network communications, [HaSLL](https://git.hahn-schickard.de/software-sollutions/application-engineering/internal/hasll) as the main logging interface, [PugiXML](https://pugixml.org/) as the XML parser library and provides all of the required dependency managment throught [Conan Dependency Mangment System](https://conan.io/). 

### Required dependencies
* [Python 3.7](https://www.python.org/downloads/release/python-370/)
* [conan](https://docs.conan.io/en/latest/installation.html)