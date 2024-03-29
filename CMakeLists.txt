cmake_minimum_required(VERSION 3.10)

project(microstl)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(HEADER_FILES "include/microstl.h")

add_executable(tests "tests/tests.cpp" ${HEADER_FILES})
target_include_directories(tests PUBLIC include)

add_executable(minimal_example "examples/minimal_example.cpp" ${HEADER_FILES})
target_include_directories(minimal_example PUBLIC include)

add_executable(custom_handler "examples/custom_handler.cpp" ${HEADER_FILES})
target_include_directories(custom_handler PUBLIC include)

add_executable(vertex_deduplication "examples/vertex_deduplication.cpp" ${HEADER_FILES})
target_include_directories(vertex_deduplication PUBLIC include)

add_executable(a2b_converter "examples/a2b_converter.cpp" ${HEADER_FILES})
target_include_directories(a2b_converter PUBLIC include)

add_test(NAME microstl COMMAND tests)
add_test(NAME minimal_example COMMAND minimal_example ${PROJECT_SOURCE_DIR}/testdata/simple_ascii.stl)
add_test(NAME custom_handler COMMAND custom_handler ${PROJECT_SOURCE_DIR}/testdata/simple_ascii.stl)
add_test(NAME vertex_deduplication COMMAND vertex_deduplication ${PROJECT_SOURCE_DIR}/testdata/box_meshlab_ascii.stl)
add_test(NAME a2b_converter COMMAND a2b_converter ${PROJECT_SOURCE_DIR}/testdata/simple_ascii.stl)
