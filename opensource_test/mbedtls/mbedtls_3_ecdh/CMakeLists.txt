add_executable("mbedtls_3_ecdh" ${CMAKE_CURRENT_SOURCE_DIR}/mbedtls_3_ecdh.c)
target_link_libraries("mbedtls_3_ecdh" PRIVATE mbedtls)
target_include_directories("mbedtls_3_ecdh" PRIVATE ${CMAKE_SOURCE_DIR}/ext/mbedtls/include)

add_executable("mbedtls_3_ecdh_server" ${CMAKE_CURRENT_SOURCE_DIR}/mbedtls_3_ecdh_server.c)
target_link_libraries("mbedtls_3_ecdh_server" PRIVATE mbedtls)
target_include_directories("mbedtls_3_ecdh_server" PRIVATE ${CMAKE_SOURCE_DIR}/ext/mbedtls/include)

add_executable("mbedtls_3_ecdh_client" ${CMAKE_CURRENT_SOURCE_DIR}/mbedtls_3_ecdh_client.c)
target_link_libraries("mbedtls_3_ecdh_client" PRIVATE mbedtls)
target_include_directories("mbedtls_3_ecdh_client" PRIVATE ${CMAKE_SOURCE_DIR}/ext/mbedtls/include)

