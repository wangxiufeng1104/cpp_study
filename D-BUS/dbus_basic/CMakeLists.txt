add_executable(dbus_server dbus_server.cpp)
target_include_directories(dbus_server PRIVATE ${DBUS_INCLUDE_DIRS})
target_compile_options(dbus_server PRIVATE ${DBUS_CFLAGS_OTHER})
target_link_libraries(dbus_server PRIVATE ${DBUS_LIBRARIES})

add_executable(dbus_client dbus_client.cpp)
target_include_directories(dbus_client PRIVATE ${DBUS_INCLUDE_DIRS})
target_compile_options(dbus_client PRIVATE ${DBUS_CFLAGS_OTHER})
target_link_libraries(dbus_client PRIVATE ${DBUS_LIBRARIES})
