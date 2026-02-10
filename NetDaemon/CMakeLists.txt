
cmake_minimum_required(VERSION 3.16)
project(NetDaemon VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

# 头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 源文件
set(SOURCES
    src/NetworkInterfaceManager.cpp
    src/RouteTableManager.cpp
    src/FirewallManager.cpp
    src/DNSManager.cpp
    src/NetworkPolicyManager.cpp
    src/NetworkMonitor.cpp
    src/main.cpp
)

# 头文件
set(HEADERS
    include/NetworkInterfaceManager.h
    include/RouteTableManager.h
    include/FirewallManager.h
    include/DNSManager.h
    include/NetworkPolicyManager.h
    include/NetworkMonitor.h
)

# 创建可执行文件
add_executable(NetDaemon ${SOURCES} ${HEADERS})

# 线程库支持
find_package(Threads REQUIRED)
target_link_libraries(NetDaemon PRIVATE Threads::Threads)

# 安装规则
install(TARGETS NetDaemon DESTINATION bin)
install(FILES ${HEADERS} DESTINATION include/NetDaemon)

# 打印配置信息
message(STATUS "NetDaemon Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  CXX Flags: ${CMAKE_CXX_FLAGS}")