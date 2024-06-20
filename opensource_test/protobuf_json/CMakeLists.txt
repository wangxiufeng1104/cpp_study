string(REGEX REPLACE "/$" "" CURRENT_FOLDER_ABSOLUTE ${CMAKE_CURRENT_SOURCE_DIR})
string(REGEX REPLACE ".*/(.*)" "\\1" CURRENT_FOLDER ${CURRENT_FOLDER_ABSOLUTE})
set (APP ${CURRENT_FOLDER})

generate_protobuf_sources()
aux_source_directory (${CMAKE_CURRENT_SOURCE_DIR} SRC)
# 添加生成文件到源码列表
list(APPEND SRC ${PROTOBUF_SRC})
add_executable(${APP} ${SRC})
target_link_libraries(${APP} protobuf pthread)
target_include_directories(${APP} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

