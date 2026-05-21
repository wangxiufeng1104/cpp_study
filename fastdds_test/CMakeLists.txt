cmake_minimum_required(VERSION 3.16)
project(fastdds_test VERSION 1.0.0 LANGUAGES CXX)

# C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 编译选项
add_compile_options(-Wall -Wextra -Wpedantic)

# ============================================================================
# 依赖管理 - 使用FetchContent自动下载FastDDS及其依赖
# ============================================================================
include(FetchContent)
set(FETCH_CONTENT_QUIET OFF)

# FastDDS全局配置 - 必须在FetchContent之前设置
set(SECURITY OFF CACHE BOOL "" FORCE)
set(SSL_AUTOCOMPLETE OFF CACHE BOOL "" FORCE)
set(COMPILE_TOOLS ON CACHE BOOL "" FORCE)  # 启用工具生成(fastddsgen)
set(INTERNAL_DEBUG OFF CACHE BOOL "" FORCE)
set(EPROSIMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(EPROSIMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# 1. Fast-CDR (FastDDS依赖，必须先编译)
FetchContent_Declare(
    fastcdr
    GIT_REPOSITORY https://github.com/eProsima/Fast-CDR.git
    GIT_TAG v2.2.0
)
FetchContent_MakeAvailable(fastcdr)

# 2. Fast DDS
FetchContent_Declare(
    fastrtps
    GIT_REPOSITORY https://github.com/eProsima/Fast-DDS.git
    GIT_TAG v3.1.0
)
FetchContent_MakeAvailable(fastrtps)

# ============================================================================
# 公共库
# ============================================================================
add_library(fastdds_test_common STATIC
    common/src/Common.cpp
)
target_include_directories(fastdds_test_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/common/include
)
target_link_libraries(fastdds_test_common PUBLIC fastrtps fastcdr)

# ============================================================================
# IDL生成函数
# ============================================================================
function(fastdds_idl_gen)
    set(options)
    set(oneValueArgs IDL_FILE OUTPUT_DIR LANG)
    set(multiValueArgs)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    get_filename_component(IDL_NAME ${ARG_IDL_FILE} NAME_WE)
    set(GENERATED_SOURCES
        ${ARG_OUTPUT_DIR}/${IDL_NAME}.cxx
        ${ARG_OUTPUT_DIR}/${IDL_NAME}CdrAux.hpp
        ${ARG_OUTPUT_DIR}/${IDL_NAME}CdrAux.ipp
        ${ARG_OUTPUT_DIR}/${IDL_NAME}PubSubTypes.cxx
        ${ARG_OUTPUT_DIR}/${IDL_NAME}PubSubTypes.hpp
        ${ARG_OUTPUT_DIR}/${IDL_NAME}.hpp
    )

    add_custom_command(
        OUTPUT ${GENERATED_SOURCES}
        COMMAND fastddsgen
            -d ${ARG_OUTPUT_DIR}
            -language ${ARG_LANG}
            -replace
            ${ARG_IDL_FILE}
        DEPENDS ${ARG_IDL_FILE} fastddsgen
        COMMENT "Generating ${IDL_NAME} types..."
    )
endfunction()

# ============================================================================
# 示例子目录
# ============================================================================
add_subdirectory(1_hello_world)
add_subdirectory(2_custom_types)
add_subdirectory(3_request_reply)
add_subdirectory(4_qos_config)
add_subdirectory(5_data_sharing)
add_subdirectory(6_tcp_transport)
add_subdirectory(7_monitoring)
add_subdirectory(8_discovery)
add_subdirectory(9_security)

# ============================================================================
# 安装
# ============================================================================
install(TARGETS fastdds_test_common
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)