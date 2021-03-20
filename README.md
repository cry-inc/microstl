# microstl

![CI](https://github.com/cry-inc/microstl/workflows/CMake/badge.svg?branch=master)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

A small C++ library for parsing STL mesh files.

## Features

* Supports ASCII and binary STL files
* Header-only library, no compilation required
* Single file, easy to add to your project
* Does not depend on any third party libraries
* Works well with your existing mesh data structures
* The parser itself has constant memory requirements (which are low)
* Tested with Visual Studio, GCC and Clang
* CMake project for tests and examples

## Quickstart

1. Download the header file `microstl.h`
2. Put the header file into your include directory
3. Add `#include <microstl.h>` to your source code
4. Create a handler that receives the parsed data and creates a mesh `microstl::MeshParserHandler meshHandler;`
5. Call the parser wiith the STL data and the handler as arguments: `microstl::Parser::parseStlFile("path/to/my.stl", meshHandler);`
6. Check return value of the parser for `microstl::Result::Success` and handle any errors 
7. Access your extracted mesh: `auto mesh = meshHandler.mesh;`

Check out the `examples` folder for complete code examples.
The file `custom_handler.cpp` shows how to write your own handler that can be used to fill your existing mesh data structues.

## Limitations

* Requires at least C++17
* Binary parser works only on little-endian systems
* Only contains a parser, no writer
