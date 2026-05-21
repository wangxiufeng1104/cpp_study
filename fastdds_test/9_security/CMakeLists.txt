# 9_security CMakeLists.txt - 安全机制示例

find_package(fastdds 3.1.0 REQUIRED)

set(IDL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../1_hello_world/HelloWorld.idl)
set(IDL_GENERATED_DIR ${CMAKE_BINARY_DIR}/idl_generated/9_security)

fastdds_idl_gen(
    IDL_FILE ${IDL_FILE}
    OUTPUT_DIR ${IDL_GENERATED_DIR}
    LANG CXX
)

include_directories(${IDL_GENERATED_DIR})

add_executable(secure_publisher publisher.cpp)
target_link_libraries(secure_publisher fastrtps fastcdr fastdds_test_common)

add_executable(secure_subscriber subscriber.cpp)
target_link_libraries(secure_subscriber fastrtps fastcdr fastdds_test_common)

install(TARGETS secure_publisher secure_subscriber DESTINATION bin)