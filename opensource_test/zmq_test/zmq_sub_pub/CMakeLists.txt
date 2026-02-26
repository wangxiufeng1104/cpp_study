# 现代方式：从目录名获取目标名
get_filename_component(APP ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# 生成 protobuf 源文件
generate_protobuf_sources(${CMAKE_CURRENT_SOURCE_DIR})

set(PROXY_SRC ${CMAKE_CURRENT_SOURCE_DIR}/zmq_proxy.cpp)
set(PUB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/zmq_publisher.cpp)
set(SUB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/zmq_subscriber.cpp)

# 现代方式：使用函数封装
function(add_zmq_executable target_name src_files)
    list(APPEND src_files ${PROTOBUF_SRC})
    add_executable(${target_name} ${src_files})
    target_link_libraries(${target_name} 
        PRIVATE 
            shared_protobuf 
            shared_zmq 
            pthread
    )
    target_include_directories(${target_name} 
        PRIVATE 
            ${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction()

add_zmq_executable(zmq_proxy "${PROXY_SRC}")
add_zmq_executable(zmq_publisher "${PUB_SRC}")
add_zmq_executable(zmq_subscriber "${SUB_SRC}")