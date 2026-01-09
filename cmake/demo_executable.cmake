# 简化演示程序的 CMakeLists.txt
# 提供两种模式：手动设置名称和自动从目录名获取名称

# 模式1: 手动设置可执行文件名称
# 参数：
#   NAME - 可执行文件名称（必需）
#   LINK_LIBS - 需要链接的库（可选，多值）
#
# 使用示例：
#   add_demo_executable_manual(NAME srp_demo)
#   add_demo_executable_manual(NAME my_demo LINK_LIBS protobuf pthread)
function(add_demo_executable_manual)
    cmake_parse_arguments(ARG
        ""                    # 选项（无前缀的开关）
        "NAME"                # 单值参数
        "LINK_LIBS"           # 多值参数
        ${ARGN}
    )
    
    if(NOT ARG_NAME)
        message(FATAL_ERROR "add_demo_executable_manual: NAME parameter is required")
    endif()
    
    set(APP ${ARG_NAME})
    
    # 收集源文件
    file(GLOB SRC CONFIGURE_DEPENDS
        "*.cpp"
        "*.c"
    )
    
    # 创建可执行文件
    add_executable(${APP} ${SRC})
    
    # 添加依赖库（如果提供了）
    if(ARG_LINK_LIBS)
        target_link_libraries(${APP} PRIVATE ${ARG_LINK_LIBS})
    endif()
    
    # 安装
    install(TARGETS ${APP}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    )
endfunction()

# 模式2: 自动从目录名获取可执行文件名称
# 参数：
#   LINK_LIBS - 需要链接的库（可选，多值）
#
# 使用示例：
#   add_demo_executable_auto()
#   add_demo_executable_auto(LINK_LIBS sqlite3 protobuf)
function(add_demo_executable_auto)
    cmake_parse_arguments(ARG
        ""                    # 选项（无前缀的开关）
        ""                    # 单值参数
        "LINK_LIBS"           # 多值参数
        ${ARGN}
    )
    
    # 从当前目录名获取可执行文件名
    get_filename_component(APP ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    
    # 收集源文件
    file(GLOB SRC CONFIGURE_DEPENDS
        "*.cpp"
        "*.c"
    )
    
    # 创建可执行文件
    add_executable(${APP} ${SRC})
    
    # 添加依赖库（如果提供了）
    if(ARG_LINK_LIBS)
        target_link_libraries(${APP} PRIVATE ${ARG_LINK_LIBS})
    endif()
    
    # 安装
    install(TARGETS ${APP}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    )
endfunction()
