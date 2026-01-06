# Dependency Inversion Principle Demonstration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 链接common库
target_link_libraries(common INTERFACE)

# 创建依赖倒置原则演示程序
add_executable(dip_demo dip_demo.cpp bad_design.h good_design.h)

# 链接common库
target_link_libraries(dip_demo PRIVATE common)

# 设置输出目录
set_target_properties(dip_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
