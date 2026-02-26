cmake_minimum_required(VERSION 3.16)
project(cpp_study LANGUAGES C CXX)

# =============================================================================
# 现代 C++ 标准设置
# =============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # 使用 -std=c++17 而非 -std=gnu++17

# =============================================================================
# 输出目录设置（现代方式）
# =============================================================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# =============================================================================
# 构建类型
# =============================================================================
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

# Release 模式下优化体积（等同于 -s）
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
    endif()
endif()

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_VERBOSE_MAKEFILE: ${CMAKE_VERBOSE_MAKEFILE}")

# =============================================================================
# 加载 CMake 模块
# =============================================================================
include(cmake/util.cmake)
include(cmake/demo_executable.cmake)
include(cmake/protobuf_helpers.cmake)

# =============================================================================
# 共享库路径（用于预编译的第三方库）
# 注意：现代 CMake 推荐使用 target-specific 的方式，这里保留全局设置
# 是因为项目使用了预编译的静态库
# =============================================================================
set(EXECUTABLE_TOOL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/shared/bin)

# 创建共享库的 imported target（现代方式）
# 这样子目录可以使用 target_link_libraries 链接这些库
add_library(shared_jsoncpp STATIC IMPORTED GLOBAL)
set_target_properties(shared_jsoncpp PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/shared/libs/libjsoncpp.a
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/shared/include
)

add_library(shared_protobuf STATIC IMPORTED GLOBAL)
set_target_properties(shared_protobuf PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/shared/libs/libprotobuf.a
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/shared/include
)

add_library(shared_protobuf_lite STATIC IMPORTED GLOBAL)
set_target_properties(shared_protobuf_lite PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/shared/libs/libprotobuf-lite.a
)

add_library(shared_zmq STATIC IMPORTED GLOBAL)
set_target_properties(shared_zmq PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/shared/libs/libzmq.a
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/shared/include
)

# rapidjson 是 header-only 库，只需要头文件目录
add_library(shared_rapidjson INTERFACE IMPORTED GLOBAL)
target_include_directories(shared_rapidjson INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/shared/include)

# =============================================================================
# 子目录
# =============================================================================
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cpp_test)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/design_patterns)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/opensource_test)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Principles_of_Code_Design)