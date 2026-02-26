# 现代方式：从目录名获取目标名
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME CURRENT_FOLDER)
set(LIB ${CURRENT_FOLDER})

# 生成 protobuf 源文件
generate_protobuf_sources(${CMAKE_CURRENT_SOURCE_DIR}/..)

# 收集源文件
file(GLOB SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
list(APPEND SRC ${PROTOBUF_SRC})

# 创建静态库
add_library(${LIB} STATIC ${SRC})

# 链接库（使用现代 target 方式）
target_link_libraries(${LIB} 
    PUBLIC 
        shared_protobuf 
        shared_zmq 
        pthread
)

# 头文件目录
target_include_directories(${LIB} 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
)