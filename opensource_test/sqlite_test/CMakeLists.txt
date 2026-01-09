# 包含 sqlite3 库
include(${CMAKE_SOURCE_DIR}/cmake/sqlite3.cmake)

# 显式添加测试子目录
add_subdirectory(sqlite3_bind)
add_subdirectory(dynamic_sql)
