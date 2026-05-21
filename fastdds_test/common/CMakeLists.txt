# 公共模块 CMakeLists.txt

# 公共源文件
set(COMMON_SOURCES
    src/Common.cpp
)

# 创建公共库
add_library(fastdds_test_common STATIC ${COMMON_SOURCES})

# 链接fastdds库
target_link_libraries(fastdds_test_common
    fastrtps
    fastcdr
)

# 设置包含目录
target_include_directories(fastdds_test_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)