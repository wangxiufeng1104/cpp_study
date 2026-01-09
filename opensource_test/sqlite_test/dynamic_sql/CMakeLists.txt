get_filename_component(APP ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_demo_executable_manual(NAME ${APP} LINK_LIBS sqlite3)