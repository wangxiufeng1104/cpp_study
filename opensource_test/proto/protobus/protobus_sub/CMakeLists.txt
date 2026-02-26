# 现代方式：从目录名获取目标名
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME CURRENT_FOLDER)
set(APP ${CURRENT_FOLDER})

# 生成 protobuf 源文件
generate_protobuf_sources(${CMAKE_CURRENT_SOURCE_DIR}/..)

# 收集源文件
file(GLOB SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
list(APPEND SRC ${PROTOBUF_SRC})

# 创建可执行文件
add_executable(${APP} ${SRC})

# 链接库（protobus_v2 会自动传递 shared_protobuf 和 shared_zmq）
target_link_libraries(${APP} 
    PRIVATE 
        sys_utils 
        protobus_v2
)