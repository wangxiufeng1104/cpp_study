cmake_minimum_required(VERSION 3.20)
project(print-helpers VERSION 1.0)
include(CMakePrintHelpers)
cmake_print_properties(DIRECTORIES .
    PROPERTIES
    BINARY_DIR
    SOURCE_DIR
)