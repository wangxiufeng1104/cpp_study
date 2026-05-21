# 8_discovery CMakeLists.txt - 发现机制示例

find_package(fastdds 3.1.0 REQUIRED)

set(IDL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../1_hello_world/HelloWorld.idl)
set(IDL_GENERATED_DIR ${CMAKE_BINARY_DIR}/idl_generated/8_discovery)

fastdds_idl_gen(
    IDL_FILE ${IDL_FILE}
    OUTPUT_DIR ${IDL_GENERATED_DIR}
    LANG CXX
)

include_directories(${IDL_GENERATED_DIR})

add_executable(discovery_participant discovery_participant.cpp)
target_link_libraries(discovery_participant fastrtps fastcdr fastdds_test_common)

install(TARGETS discovery_participant DESTINATION bin)