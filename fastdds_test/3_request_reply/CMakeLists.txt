# 3_request_reply CMakeLists.txt - 请求应答模式示例

find_package(fastdds 3.1.0 REQUIRED)

set(IDL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/idl/RequestReply.idl)
set(IDL_GENERATED_DIR ${CMAKE_BINARY_DIR}/idl_generated/3_request_reply)

fastdds_idl_gen(
    IDL_FILE ${IDL_FILE}
    OUTPUT_DIR ${IDL_GENERATED_DIR}
    LANG CXX
)

include_directories(${IDL_GENERATED_DIR})

add_executable(request_reply_requester requester.cpp)
target_link_libraries(request_reply_requester fastrtps fastcdr fastdds_test_common)

add_executable(request_reply_replier replier.cpp)
target_link_libraries(request_reply_replier fastrtps fastcdr fastdds_test_common)

install(TARGETS request_reply_requester request_reply_replier DESTINATION bin)