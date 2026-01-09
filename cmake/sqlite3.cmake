# SQLite3 (amalgamation) 集成
# - 将 sqlite3 构建为自包含的静态库目标
# - 向下游目标隐藏第三方源码布局
# - 适用于嵌入式 / 离线构建

include(FetchContent)

FetchContent_Declare(
    sqlite3_src
    URL https://www.sqlite.org/2024/sqlite-amalgamation-3460100.zip
    # 将 SOURCE_DIR 固定，以确保第三方头文件对 IDE（如 VS Code / clangd）可见
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/ext/sqlite3 # 指定库下载地址
)

FetchContent_MakeAvailable(sqlite3_src)

add_library(sqlite3 STATIC
    ${sqlite3_src_SOURCE_DIR}/sqlite3.c
)

# sqlite3.h 是公共接口的一部分
target_include_directories(sqlite3
    PUBLIC ${sqlite3_src_SOURCE_DIR}
)
