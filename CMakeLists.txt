cmake_minimum_required(VERSION 3.16)
project(cpp_study)

include(cmake/util.cmake)
include(cmake/protobuf_helpers.cmake)
include(cmake/mbedtls_2_7.cmake)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=libstdc++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17") # set C++ 11
#set the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)
if(CMAKE_BUILD_TYPE MATCHES Release)
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
# 是否打开编译细节
if(NOT CMAKE_VERBOSE_MAKEFILE)
    message(STATUS "CMAKE_VERBOSE_MAKEFILE: ${CMAKE_VERBOSE_MAKEFILE}")
    set(CMAKE_VERBOSE_MAKEFILE OFF)
endif()


set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin)
set(EXECUTABLE_TOOL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/shared/bin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/shared/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/shared/libs)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cpp_test) 
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/design_patterns)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/opensource_test)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Principles_of_Code_Design)
