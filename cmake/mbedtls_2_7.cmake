include(FetchContent)

# FetchContent_Declare(mbedtls
#         GIT_REPOSITORY https://github.com/Mbed-TLS/mbedtls.git
#         GIT_TAG v3.6.2)

fetchcontent_declare(mbedtls
        URL https://github.com/Mbed-TLS/mbedtls/archive/refs/tags/v2.7.16.zip
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/ext/mbedtls # 指定库下载地址
        )
# 设置选项以禁用测试和示例代码
set(ENABLE_TESTING OFF CACHE BOOL "Disable building testing code")
set(ENABLE_PROGRAMS OFF CACHE BOOL "Disable building example code")
fetchcontent_makeavailable(mbedtls)