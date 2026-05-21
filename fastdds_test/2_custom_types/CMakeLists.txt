# 2_custom_types CMakeLists.txt - 自定义数据类型示例

find_package(fastdds 3.1.0 REQUIRED)

# 生成IDL源文件
set(IDL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/idl/CustomData.idl)
set(IDL_GENERATED_DIR ${CMAKE_BINARY_DIR}/idl_generated/2_custom_types)

fastdds_idl_gen(
    IDL_FILE ${IDL_FILE}
    OUTPUT_DIR ${IDL_GENERATED_DIR}
    LANG CXX
)

include_directories(${IDL_GENERATED_DIR})

# 发布者
add_executable(custom_types_publisher publisher.cpp)
target_link_libraries(custom_types_publisher fastrtps fastcdr fastdds_test_common)

# 订阅者
add_executable(custom_types_subscriber subscriber.cpp)
target_link_libraries(custom_types_subscriber fastrtps fastcdr fastdds_test_common)

install(TARGETS custom_types_publisher custom_types_subscriber DESTINATION bin)