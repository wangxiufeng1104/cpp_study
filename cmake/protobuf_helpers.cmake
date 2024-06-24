# protobuf_helpers.cmake

function(generate_protobuf_sources PROTO_PATH)
    # 查找所有 .proto 文件
    file(GLOB PROTO_FILES "${PROTO_PATH}/*.proto")

    # 设置生成的源文件和头文件列表
    set(PROTOBUF_SRC_LIST)
    set(PROTOBUF_HDR_LIST)

    # 遍历每个 .proto 文件并生成相应的 .pb.cc 和 .pb.h 文件
    foreach(PROTO_FILE ${PROTO_FILES})
        get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
        set(PROTO_SRC "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc")
        set(PROTO_HDR "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h")
        list(APPEND PROTOBUF_SRC_LIST "${PROTO_SRC}")
        list(APPEND PROTOBUF_HDR_LIST "${PROTO_HDR}")
        add_custom_command(
            OUTPUT ${PROTO_SRC} ${PROTO_HDR}
            COMMAND ${EXECUTABLE_TOOL_PATH}/protoc-3.21.7.0
            ARGS -I=${PROTO_PATH}/proto --cpp_out=${CMAKE_CURRENT_BINARY_DIR} --proto_path=${PROTO_PATH} ${PROTO_FILE}
            DEPENDS ${PROTO_FILE}
        )
    endforeach()

    set(PROTOBUF_SRC ${PROTOBUF_SRC_LIST} PARENT_SCOPE)
    set(PROTOBUF_HDR ${PROTOBUF_HDR_LIST} PARENT_SCOPE)
endfunction()
