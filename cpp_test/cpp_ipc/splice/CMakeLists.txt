

add_executable("splice_server" ${CMAKE_CURRENT_SOURCE_DIR}/splice_server.cpp)
add_executable("splice_client" ${CMAKE_CURRENT_SOURCE_DIR}/splice_client.cpp)

install(TARGETS "splice_server" DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(TARGETS "splice_client" DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)