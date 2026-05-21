# 5_data_sharing CMakeLists.txt - DataSharing共享内存通信示例

find_package(fastdds 3.1.0 REQUIRED)

set(IDL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../1_hello_world/HelloWorld.idl)
set(IDL_GENERATED_DIR ${CMAKE_BINARY_DIR}/idl_generated/5_data_sharing)

fastdds_idl_gen(
    IDL_FILE ${IDL_FILE}
    OUTPUT_DIR ${IDL_GENERATED_DIR}
    LANG CXX
)

include_directories(${IDL_GENERATED_DIR})

add_executable(data_sharing_publisher publisher.cpp)
target_link_libraries(data_sharing_publisher fastrtps fastcdr fastdds_test_common)

add_executable(data_sharing_subscriber subscriber.cpp)
target_link_libraries(data_sharing_subscriber fastrtps fastcdr fastdds_test_common)

install(TARGETS data_sharing_publisher data_sharing_subscriber DESTINATION bin)