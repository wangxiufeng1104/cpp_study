# List of targets
set(MBEDTLS_TARGETS
    mbedtls_3_ecdh
    mbedtls_3_ecdh_server
    mbedtls_3_ecdh_client
)

# Add executables and link libraries
foreach(target IN LISTS MBEDTLS_TARGETS)
    add_executable(${target} ${CMAKE_CURRENT_SOURCE_DIR}/${target}.c)
    target_link_libraries(${target} PRIVATE mbedtls)
    target_include_directories(${target} PRIVATE ${mbedtls_SOURCE_DIR}/include)
endforeach()

