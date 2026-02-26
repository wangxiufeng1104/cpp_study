# 现代方式：从目录名获取目标名
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME CURRENT_FOLDER)
set(APP ${CURRENT_FOLDER})

# 收集源文件
file(GLOB SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

# 创建可执行文件
add_executable(${APP} ${SRC})

# 链接库（使用现代 target 方式，shared_zmq 会自动传递 include 目录）
target_link_libraries(${APP} 
    PRIVATE 
        sys_utils 
        pthread 
        shared_zmq
)