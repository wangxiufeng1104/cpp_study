# CMake 现代化改造文档

## 概述

本文档描述了对 `cpp_study` 项目 CMake 配置的现代化改造，使其符合 CMake 3.16+ 的最佳实践。

## 改造背景

原始的 CMakeLists.txt 存在以下问题：

1. **使用已弃用的全局命令**：`include_directories()`、`link_directories()`
2. **重复设置 C++ 标准**：同时使用 `CMAKE_CXX_STANDARD` 和手动修改 `CMAKE_CXX_FLAGS`
3. **旧式输出路径变量**：使用 `LIBRARY_OUTPUT_PATH`、`EXECUTABLE_OUTPUT_PATH`
4. **直接使用库名而非 target**：如 `jsoncpp`、`protobuf`、`zmq`

## 现代 CMake 核心原则

### Target-based 依赖管理

现代 CMake 的核心理念是 **以目标（target）为中心** 的依赖管理：

```cmake
# ❌ 旧式做法（全局设置）
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
target_link_libraries(my_app jsoncpp zmq)

# ✅ 现代做法（target-specific）
add_library(shared_jsoncpp STATIC IMPORTED GLOBAL)
set_target_properties(shared_jsoncpp PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/libjsoncpp.a
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(my_app PRIVATE shared_jsoncpp)
```

### 可见性修饰符

| 修饰符 | 含义 | 使用场景 |
|--------|------|----------|
| `PRIVATE` | 仅当前目标使用 | 实现细节依赖 |
| `PUBLIC` | 当前目标和依赖者都使用 | 接口依赖（头文件中暴露） |
| `INTERFACE` | 仅依赖者使用 | header-only 库 |

## 主要改动

### 1. 主 CMakeLists.txt

#### 1.1 语言声明

```cmake
# 添加 C 语言支持（mbedtls 需要）
project(cpp_study LANGUAGES C CXX)
```

#### 1.2 C++ 标准设置

```cmake
# 现代 C++ 标准设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)   # 要求编译器支持 C++17
set(CMAKE_CXX_EXTENSIONS OFF)          # 使用 -std=c++17 而非 -std=gnu++17
```

#### 1.3 输出目录

```cmake
# 现代输出目录变量
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
```

#### 1.4 Imported Targets

为预编译的共享库创建 imported targets：

```cmake
# 静态库示例
add_library(shared_jsoncpp STATIC IMPORTED GLOBAL)
set_target_properties(shared_jsoncpp PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/shared/libs/libjsoncpp.a
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/shared/include
)

# Header-only 库示例（rapidjson）
add_library(shared_rapidjson INTERFACE IMPORTED GLOBAL)
target_include_directories(shared_rapidjson INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/shared/include)
```

**可用的 Imported Targets：**

| Target 名称 | 类型 | 说明 |
|-------------|------|------|
| `shared_jsoncpp` | STATIC IMPORTED | JsonCpp JSON 库 |
| `shared_protobuf` | STATIC IMPORTED | Google Protocol Buffers |
| `shared_protobuf_lite` | STATIC IMPORTED | Protobuf Lite 版本 |
| `shared_zmq` | STATIC IMPORTED | ZeroMQ 消息队列 |
| `shared_rapidjson` | INTERFACE IMPORTED | RapidJSON（header-only） |

### 2. 子目录 CMakeLists.txt 模板

#### 2.1 简单可执行文件

```cmake
# 现代方式：从目录名获取目标名
get_filename_component(APP ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# 收集源文件
file(GLOB SRC CONFIGURE_DEPENDS "*.cpp" "*.c")

# 创建可执行文件
add_executable(${APP} ${SRC})

# 链接库（使用现代 target 方式）
target_link_libraries(${APP} 
    PRIVATE 
        shared_jsoncpp
)

# 添加头文件目录（如果需要）
target_include_directories(${APP} 
    PRIVATE 
        ${CMAKE_CURRENT_BINARY_DIR}
)
```

#### 2.2 静态库

```cmake
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME CURRENT_FOLDER)
set(LIB ${CURRENT_FOLDER})

# 收集源文件
file(GLOB SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# 创建静态库
add_library(${LIB} STATIC ${SRC})

# 链接库（PUBLIC 会传递给依赖者）
target_link_libraries(${LIB} 
    PUBLIC 
        shared_protobuf 
        shared_zmq 
        pthread
)

# 头文件目录
target_include_directories(${LIB} 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE 
        ${CMAKE_CURRENT_BINARY_DIR}
)
```

#### 2.3 使用辅助函数

项目提供了 `add_demo_executable_auto()` 和 `add_demo_executable_manual()` 函数：

```cmake
# 自动从目录名获取可执行文件名
add_demo_executable_auto(LINK_LIBS shared_jsoncpp)

# 手动指定名称
add_demo_executable_manual(NAME my_app LINK_LIBS shared_protobuf pthread)
```

## 文件修改清单

| 文件路径 | 主要改动 |
|----------|----------|
| `CMakeLists.txt` | 全面现代化改造，添加 imported targets |
| `opensource_test/jsoncpp_test/CMakeLists.txt` | 使用 `shared_jsoncpp` |
| `opensource_test/rapidjson_test/CMakeLists.txt` | 使用 `shared_rapidjson` |
| `opensource_test/zmq_test/zmq_basic/CMakeLists.txt` | 使用 `shared_zmq` |
| `opensource_test/zmq_test/zmq_sub_pub/CMakeLists.txt` | 使用 `shared_protobuf`、`shared_zmq` |
| `opensource_test/proto/contact/CMakeLists.txt` | 使用 `shared_protobuf` |
| `opensource_test/proto/protobuf_test/CMakeLists.txt` | 使用 `shared_protobuf` |
| `opensource_test/proto/protobuf_json/CMakeLists.txt` | 使用 `shared_protobuf` |
| `opensource_test/proto/protobus/protobus_v1/CMakeLists.txt` | 使用 `shared_protobuf`、`shared_zmq` |
| `opensource_test/proto/protobus/protobus_v2/CMakeLists.txt` | 使用 `shared_protobuf`、`shared_zmq` |
| `opensource_test/proto/protobus/protobus_pub/CMakeLists.txt` | 依赖传递优化 |
| `opensource_test/proto/protobus/protobus_sub/CMakeLists.txt` | 依赖传递优化 |
| `opensource_test/proto/protobus/protobus_node_cpp/CMakeLists.txt` | 依赖传递优化 |
| `opensource_test/proto/protobus/protobus_proxy/CMakeLists.txt` | 使用 `shared_zmq` |
| `opensource_test/proto/protobus/spdlog/CMakeLists.txt` | 依赖传递优化 |

## 最佳实践总结

### ✅ 推荐做法

1. **使用 target-based 命令**
   ```cmake
   target_include_directories(target PRIVATE ...)
   target_link_libraries(target PRIVATE ...)
   target_compile_features(target PRIVATE cxx_std_17)
   ```

2. **使用 IMPORTED targets 管理外部库**
   ```cmake
   add_library(foo STATIC IMPORTED GLOBAL)
   set_target_properties(foo PROPERTIES
       IMPORTED_LOCATION "/path/to/libfoo.a"
       INTERFACE_INCLUDE_DIRECTORIES "/path/to/include"
   )
   ```

3. **明确指定可见性**
   ```cmake
   target_link_libraries(my_app 
       PRIVATE internal_lib
       PUBLIC interface_lib
   )
   ```

4. **使用 `file(GLOB CONFIGURE_DEPENDS)`**
   ```cmake
   file(GLOB SRC CONFIGURE_DEPENDS "src/*.cpp")
   ```

### ❌ 避免做法

1. **避免全局命令**
   ```cmake
   # ❌ 不要使用
   include_directories(...)
   link_directories(...)
   ```

2. **避免直接修改编译标志设置标准**
   ```cmake
   # ❌ 不要使用
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
   
   # ✅ 应该使用
   set(CMAKE_CXX_STANDARD 17)
   ```

3. **避免使用旧式输出路径变量**
   ```cmake
   # ❌ 不要使用
   set(EXECUTABLE_OUTPUT_PATH ...)
   set(LIBRARY_OUTPUT_PATH ...)
   ```

## 构建说明

```bash
# 配置项目
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 构建
cmake --build build -j4

# 或使用传统方式
cd build
cmake ..
make -j4
```

## 参考资料

- [CMake 3.16 官方文档](https://cmake.org/cmake/help/v3.16/)
- [Modern CMake 开始指南](https://cliutils.gitlab.io/modern-cmake/)
- [Effective CMake](https://www.youtube.com/watch?v=bsXLMQ6Wg-I)