# 4_qos_config CMakeLists.txt - QoS配置示例

find_package(fastdds 3.1.0 REQUIRED)

set(IDL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../1_hello_world/HelloWorld.idl)
set(IDL_GENERATED_DIR ${CMAKE_BINARY_DIR}/idl_generated/4_qos_config)

fastdds_idl_gen(
    IDL_FILE ${IDL_FILE}
    OUTPUT_DIR ${IDL_GENERATED_DIR}
    LANG CXX
)

include_directories(${IDL_GENERATED_DIR})

add_executable(qos_publisher publisher.cpp)
target_link_libraries(qos_publisher fastrtps fastcdr fastdds_test_common)

add_executable(qos_subscriber subscriber.cpp)
target_link_libraries(qos_subscriber fastrtps fastcdr fastdds_test_common)

install(TARGETS qos_publisher qos_subscriber DESTINATION bin)