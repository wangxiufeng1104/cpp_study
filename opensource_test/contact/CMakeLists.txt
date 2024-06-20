string(REGEX REPLACE "/$" "" CURRENT_FOLDER_ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR})
string(REGEX REPLACE ".*/(.*)" "\\1" CURRENT_FOLDER ${CURRENT_FOLDER_ABSOLUTE})
set (APP ${CURRENT_FOLDER})

# 查找所有 .proto 文件
file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")
# 设置生成的源文件和头文件列表
set(GENERATED_SRC)
set(GENERATED_HDR)

# 遍历每个 .proto 文件并生成相应的 .pb.cc 和 .pb.h 文件
foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    list(APPEND GENERATED_SRC "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc")
    list(APPEND GENERATED_HDR "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h")
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h
        COMMAND ${EXECUTABLE_TOOL_PATH}/protoc-3.21.7.0
        ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR} --proto_path=${CMAKE_CURRENT_SOURCE_DIR} ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
    )
endforeach()
aux_source_directory (${CMAKE_CURRENT_SOURCE_DIR} SRC)
# 添加生成文件到源码列表
list(APPEND SRC ${GENERATED_SRC})
add_executable(${APP} ${SRC})
target_link_libraries(${APP} protobuf pthread)
target_include_directories(${APP} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

