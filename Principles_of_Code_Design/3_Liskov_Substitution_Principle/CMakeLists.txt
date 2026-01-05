# Liskov Substitution Principle Demonstration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 链接common库
target_link_libraries(common INTERFACE)

# 创建里氏替换原则演示程序
add_executable(lsp_demo lsp_demo.cpp bad_design.h good_design.h)

# 链接common库
target_link_libraries(lsp_demo PRIVATE common)

# 设置输出目录
set_target_properties(lsp_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
