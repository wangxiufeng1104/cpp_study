cmake_minimum_required(VERSION 3.16)
project(dbus_learning LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED dbus-1)

message(STATUS "DBUS include dirs: ${DBUS_INCLUDE_DIRS}")
message(STATUS "DBUS libraries:    ${DBUS_LIBRARIES}")
message(STATUS "DBUS cflags other: ${DBUS_CFLAGS_OTHER}")

add_subdirectory(dbus_basic)
add_subdirectory(dbus_signal)
add_subdirectory(dbus_properties)
add_subdirectory(dbus_activation)
