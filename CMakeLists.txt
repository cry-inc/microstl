cmake_minimum_required(VERSION 3.10)
project(microstl)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(HEADER_FILES include/microstl.h)

add_executable(development "src/development.cpp" ${HEADER_FILES})
target_include_directories(development PUBLIC include)

add_executable(tests src/tests.cpp ${HEADER_FILES})
target_include_directories(tests PUBLIC include)
