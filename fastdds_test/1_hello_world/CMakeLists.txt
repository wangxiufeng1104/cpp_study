# 1_hello_world CMakeLists.txt - 基础发布订阅示例

# 使用FastDDS IDL生成器
find_package(fastdds 3.1.0 REQUIRED)

# 生成IDL源文件
set(IDL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorld.idl)
set(IDL_GENERATED_DIR ${CMAKE_BINARY_DIR}/idl_generated/1_hello_world)

# 生成C++类型支持文件
fastdds_idl_gen(
    IDL_FILE ${IDL_FILE}
    OUTPUT_DIR ${IDL_GENERATED_DIR}
    LANG CXX
)

# 包含生成文件目录
include_directories(${IDL_GENERATED_DIR})

# ============================================================================
# 发布者可执行文件
# ============================================================================
add_executable(hello_world_publisher
    publisher.cpp
)

target_link_libraries(hello_world_publisher
    fastrtps
    fastcdr
    fastdds_test_common
)

# ============================================================================
# 订阅者可执行文件
# ============================================================================
add_executable(hello_world_subscriber
    subscriber.cpp
)

target_link_libraries(hello_world_subscriber
    fastrtps
    fastcdr
    fastdds_test_common
)

# ============================================================================
# 安装规则
# ============================================================================
install(TARGETS hello_world_publisher hello_world_subscriber
    DESTINATION bin
)