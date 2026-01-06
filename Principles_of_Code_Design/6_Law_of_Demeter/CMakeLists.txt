# Law of Demeter Demonstration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 链接common库
target_link_libraries(common INTERFACE)

# 创建迪米特法则演示程序
add_executable(lod_demo lod_demo.cpp bad_design.h good_design.h)

# 链接common库
target_link_libraries(lod_demo PRIVATE common)

# 设置输出目录
set_target_properties(lod_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
