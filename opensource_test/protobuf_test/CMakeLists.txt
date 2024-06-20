string(REGEX REPLACE "/$" "" CURRENT_FOLDER_ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR})
string(REGEX REPLACE ".*/(.*)" "\\1" CURRENT_FOLDER ${CURRENT_FOLDER_ABSOLUTE})
set (APP ${CURRENT_FOLDER})

generate_protobuf_sources()

set(WRITER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/testwrite.cpp)
set(READER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/testread.cpp)

function(add_protobuf_executable target_name src_files)
    list(APPEND src_files ${PROTOBUF_SRC})
    add_executable(${target_name} ${src_files})
    target_link_libraries(${target_name} protobuf pthread)
    target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endfunction()
add_protobuf_executable(protobuf_writer "${WRITER_SRC}")
add_protobuf_executable(protobuf_reader "${READER_SRC}")

