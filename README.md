# microstl

![CI](https://github.com/cry-inc/microstl/workflows/CMake/badge.svg?branch=master)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

A small C++ library for reading and writing STL mesh files.

## Features

* Supports ASCII and binary STL files
* Header-only library, no compilation required
* Single file, easy to add to your project
* Does not depend on any third-party libraries
* Works well with your existing mesh data structures
* Optional vertex deduplication after reading (to get a proper face-vertex data structure)
* Tested with Visual Studio, GCC and Clang
* CMake for tests and examples

## Quickstart: Reading a STL file

1. Download the header file `microstl.h`
2. Put the file into your include directory
3. Include the file: `#include <microstl.h>`
4. Create a handler that receives the data and creates a mesh: `microstl::MeshReaderHandler meshHandler;`
5. Call the reader with the STL file path and the handler: `microstl::Reader::readStlFile("path/to/my.stl", meshHandler);`
6. Check return value of the reader for `microstl::Result::Success` and handle any errors 
7. Access the triangle data in the mesh handler at `meshHandler.mesh.facets`

Check out the `examples` folder for more code examples.
The file `custom_handler.cpp` shows how to write your own handler.
Such a custom handler can be used to fill your existing mesh data structues.

The writer follows the same principle. You can use the included simple mesh data structures or
you can implement a custom data provider to connect your own data structures.

## Limitations

* Requires at least a C++ 17 compiler
* Binary reader and writer works only on little-endian systems
