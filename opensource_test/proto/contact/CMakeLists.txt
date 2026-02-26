# 现代方式：从目录名获取目标名
get_filename_component(APP ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# 生成 protobuf 源文件
generate_protobuf_sources(${CMAKE_CURRENT_SOURCE_DIR})

# 收集源文件
file(GLOB SRC CONFIGURE_DEPENDS "*.cpp" "*.c")

# 添加生成文件到源码列表
list(APPEND SRC ${PROTOBUF_SRC})

# 创建可执行文件
add_executable(${APP} ${SRC})

# 链接库（使用现代 target 方式）
target_link_libraries(${APP} 
    PRIVATE 
        shared_protobuf 
        pthread
)

# 添加头文件目录
target_include_directories(${APP} 
    PRIVATE 
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/include
)

