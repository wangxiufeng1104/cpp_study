cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME CURRENT_FOLDER)
set (APP ${CURRENT_FOLDER})

aux_source_directory (${CMAKE_CURRENT_SOURCE_DIR}/src SRC)

add_executable(${APP} ${SRC})

target_link_libraries(${APP} sys_utils pthread zmq)
