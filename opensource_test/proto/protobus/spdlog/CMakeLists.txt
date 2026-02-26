# 开启警告的辅助函数
function(spdlog_enable_warnings target_name)
    if(SPDLOG_BUILD_WARNINGS)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            list(APPEND MSVC_OPTIONS "/W3")
            if(MSVC_VERSION GREATER 1900) # Allow non fatal security warnings for msvc 2015
                list(APPEND MSVC_OPTIONS "/WX")
            endif()
        endif()

        target_compile_options(
            ${target_name}
            PRIVATE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
                    -Wall
                    -Wextra
                    -Wconversion
                    -pedantic
                    -Werror
                    -Wfatal-errors>
                    $<$<CXX_COMPILER_ID:MSVC>:${MSVC_OPTIONS}>)
    endif()
endfunction()

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

# 开启警告
spdlog_enable_warnings(${APP})

# 链接库（protobus_v2 会自动传递 shared_protobuf 和 shared_zmq）
target_link_libraries(${APP} 
    PRIVATE 
        pthread 
        protobus_v2
)

# 头文件目录
target_include_directories(${APP}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)