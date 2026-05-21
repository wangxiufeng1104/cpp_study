add_executable(data_acq_server data_acq_server.cpp)
target_include_directories(data_acq_server PRIVATE ${DBUS_INCLUDE_DIRS})
target_compile_options(data_acq_server PRIVATE ${DBUS_CFLAGS_OTHER})
target_link_libraries(data_acq_server PRIVATE ${DBUS_LIBRARIES})

add_executable(data_acq_control data_acq_control.cpp)
target_include_directories(data_acq_control PRIVATE ${DBUS_INCLUDE_DIRS})
target_compile_options(data_acq_control PRIVATE ${DBUS_CFLAGS_OTHER})
target_link_libraries(data_acq_control PRIVATE ${DBUS_LIBRARIES})

add_executable(data_acq_client data_acq_client.cpp)
target_include_directories(data_acq_client PRIVATE ${DBUS_INCLUDE_DIRS})
target_compile_options(data_acq_client PRIVATE ${DBUS_CFLAGS_OTHER})
target_link_libraries(data_acq_client PRIVATE ${DBUS_LIBRARIES})
