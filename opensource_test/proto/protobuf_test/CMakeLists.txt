# 现代方式：从目录名获取目标名
get_filename_component(APP ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# 生成 protobuf 源文件
generate_protobuf_sources(${CMAKE_CURRENT_SOURCE_DIR})

set(WRITER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/testwrite.cpp)
set(READER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/testread.cpp)

# 现代方式：使用函数封装
function(add_protobuf_executable target_name src_files)
    list(APPEND src_files ${PROTOBUF_SRC})
    add_executable(${target_name} ${src_files})
    target_link_libraries(${target_name} 
        PRIVATE 
            shared_protobuf 
            pthread
    )
    target_include_directories(${target_name} 
        PRIVATE 
            ${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction()

add_protobuf_executable(protobuf_writer "${WRITER_SRC}")
add_protobuf_executable(protobuf_reader "${READER_SRC}")