# Single Responsibility Principle Demonstration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 链接common库
target_link_libraries(common INTERFACE)

# 创建嵌入式设备演示程序（单一职责原则演示）
add_executable(single_responsibility_demo embedded_example.cpp embedded_example.h)

# 链接common库
target_link_libraries(single_responsibility_demo PRIVATE common)

# 设置输出目录
set_target_properties(single_responsibility_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
