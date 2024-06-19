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
    list(APPEND GENERATED_SRC "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${PROTO_NAME}.pb.cc")
    list(APPEND GENERATED_HDR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${PROTO_NAME}.pb.h")
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${PROTO_NAME}.pb.cc ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${PROTO_NAME}.pb.h
        COMMAND ${EXECUTABLE_TOOL_PATH}/protoc-3.21.7.0
        ARGS -I=${CMAKE_CURRENT_SOURCE_DIR}/proto --cpp_out=${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE} --proto_path=${CMAKE_CURRENT_SOURCE_DIR} ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
    )
endforeach()

set(WRITER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/testwrite.cpp)
set(READER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/testread.cpp)
# 添加生成文件到源码列表
list(APPEND WRITER_SRC ${GENERATED_SRC})
add_executable(writer ${WRITER_SRC})
target_link_libraries(writer protobuf pthread)
target_include_directories(writer PRIVATE ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})

list(APPEND READER_SRC ${GENERATED_SRC})
add_executable(reader ${READER_SRC})
target_link_libraries(reader protobuf pthread)
target_include_directories(reader PRIVATE ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
