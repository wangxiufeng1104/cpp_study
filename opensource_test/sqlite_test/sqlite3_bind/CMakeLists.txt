get_filename_component(APP ${CMAKE_CURRENT_SOURCE_DIR} NAME)

file(GLOB SRC CONFIGURE_DEPENDS
    "*.cpp"
    "*.c"
)

add_executable(${APP} ${SRC})

target_link_libraries(${APP}
    PRIVATE sqlite3
)

install(TARGETS ${APP}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)
