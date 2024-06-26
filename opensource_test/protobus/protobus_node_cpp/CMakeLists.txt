cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME CURRENT_FOLDER)
set (APP ${CURRENT_FOLDER})
generate_protobuf_sources(${CMAKE_CURRENT_SOURCE_DIR}/..)
aux_source_directory (${CMAKE_CURRENT_SOURCE_DIR}/src SRC)
list(APPEND SRC ${PROTOBUF_SRC})
add_executable(${APP} ${SRC})
target_link_libraries(${APP} sys_utils protobus_v1)
target_include_directories(${APP} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

