add_executable(dbus_signal_sender dbus_signal_sender.cpp)
target_include_directories(dbus_signal_sender PRIVATE ${DBUS_INCLUDE_DIRS})
target_compile_options(dbus_signal_sender PRIVATE ${DBUS_CFLAGS_OTHER})
target_link_libraries(dbus_signal_sender PRIVATE ${DBUS_LIBRARIES})

add_executable(dbus_signal_receiver dbus_signal_receiver.cpp)
target_include_directories(dbus_signal_receiver PRIVATE ${DBUS_INCLUDE_DIRS})
target_compile_options(dbus_signal_receiver PRIVATE ${DBUS_CFLAGS_OTHER})
target_link_libraries(dbus_signal_receiver PRIVATE ${DBUS_LIBRARIES})
