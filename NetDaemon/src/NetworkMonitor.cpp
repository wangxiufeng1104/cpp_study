#include "NetworkMonitor.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

NetworkMonitor::NetworkMonitor()
    : initialized_(false), running_(false), monitoring_interval_(5) {
    monitor_stats_.running = false;
    monitor_stats_.monitoring_interval = monitoring_interval_;
    monitor_stats_.total_events = 0;
    monitor_stats_.connection_changes = 0;
}

NetworkMonitor::~NetworkMonitor() {
    stop();
}

bool NetworkMonitor::initialize() {
    if (initialized_) {
        return true;
    }

    // 读取网络状态
    readNetworkStatus();
    readNetworkStats();

    initialized_ = true;
    return true;
}

bool NetworkMonitor::start() {
    if (running_) {
        return true;
    }

    running_ = true;
    monitor_stats_.running = true;
    std::cout << "[NetworkMonitor] Started network monitoring" << std::endl;

    // 在实际实现中，这里会启动一个监控线程
    // 定期调用 detectStateChanges() 和 refreshStatus()

    return true;
}

bool NetworkMonitor::stop() {
    if (!running_) {
        return true;
    }

    running_ = false;
    monitor_stats_.running = false;
    std::cout << "[NetworkMonitor] Stopped network monitoring" << std::endl;

    return true;
}

std::vector<NetworkConnection> NetworkMonitor::getConnections() const {
    std::vector<NetworkConnection> result;
    for (const auto& pair : connections_) {
        result.push_back(pair.second);
    }
    return result;
}

NetworkConnection* NetworkMonitor::getConnection(const std::string& network_id) {
    auto it = connections_.find(network_id);
    if (it != connections_.end()) {
        return &it->second;
    }
    return nullptr;
}

NetworkConnection* NetworkMonitor::getConnectionByInterface(const std::string& interface) {
    for (auto& pair : connections_) {
        if (pair.second.interface == interface) {
            return &pair.second;
        }
    }
    return nullptr;
}

NetworkStats NetworkMonitor::getNetworkStats(const std::string& interface) {
    auto it = stats_.find(interface);
    if (it != stats_.end()) {
        return it->second;
    }

    // 如果找不到，创建一个空的统计信息
    NetworkStats empty_stats;
    empty_stats.interface = interface;
    empty_stats.rx_bytes = 0;
    empty_stats.tx_bytes = 0;
    empty_stats.rx_packets = 0;
    empty_stats.tx_packets = 0;
    empty_stats.rx_errors = 0;
    empty_stats.tx_errors = 0;
    empty_stats.rx_dropped = 0;
    empty_stats.tx_dropped = 0;
    
    return empty_stats;
}

bool NetworkMonitor::refreshStatus() {
    readNetworkStatus();
    readNetworkStats();
    detectStateChanges();
    return true;
}

bool NetworkMonitor::checkConnectivity(const std::string& host) {
    std::cout << "[NetworkMonitor] Checking connectivity to " << host << std::endl;

    // 解析主机地址
    struct addrinfo hints, *result, *rp;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    // 支持 IPv4 和 IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP

    int ret = getaddrinfo(host.c_str(), "80", &hints, &result);
    if (ret != 0) {
        std::cerr << "[NetworkMonitor] getaddrinfo failed: " << gai_strerror(ret) << std::endl;
        return false;
    }

    bool connected = false;
    double time_ms = 0.0;
    
    // 遍历所有返回的地址
    for (rp = result; rp != nullptr; rp = rp->ai_next) {
        int sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1) {
            continue;
        }

        // 设置非阻塞模式以实现超时
        int flags = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

        // 记录开始时间
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        // 尝试连接
        ret = connect(sockfd, rp->ai_addr, rp->ai_addrlen);
        
        if (ret == -1) {
            if (errno == EINPROGRESS) {
                // 连接正在进行中，等待结果
                fd_set writefds;
                FD_ZERO(&writefds);
                FD_SET(sockfd, &writefds);
                
                struct timeval timeout;
                timeout.tv_sec = 3;  // 3秒超时
                timeout.tv_usec = 0;
                
                ret = select(sockfd + 1, nullptr, &writefds, nullptr, &timeout);
                
                if (ret > 0) {
                    // 检查连接是否成功
                    int error = 0;
                    socklen_t len = sizeof(error);
                    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) == 0 && error == 0) {
                        connected = true;
                        
                        // 计算连接时间
                        clock_gettime(CLOCK_MONOTONIC, &end);
                        time_ms = (end.tv_sec - start.tv_sec) * 1000.0 +
                                  (end.tv_nsec - start.tv_nsec) / 1000000.0;
                    }
                }
            }
        } else {
            // 立即连接成功
            connected = true;
            clock_gettime(CLOCK_MONOTONIC, &end);
            time_ms = (end.tv_sec - start.tv_sec) * 1000.0 +
                      (end.tv_nsec - start.tv_nsec) / 1000000.0;
        }

        close(sockfd);
        
        if (connected) {
            break;
        }
    }

    freeaddrinfo(result);
    
    std::cout << "[NetworkMonitor] Connectivity check to " << host << ": "
              << (connected ? "SUCCESS" : "FAILED") << std::endl;
    
    if (connected) {
        std::cout << "[NetworkMonitor] Connection time: " << time_ms << " ms" << std::endl;
    }
    
    return connected;
}

int NetworkMonitor::getSignalStrength(const std::string& interface) {
    auto* conn = getConnectionByInterface(interface);
    if (conn) {
        return conn->signal_strength;
    }
    return -1;
}

int NetworkMonitor::getLinkSpeed(const std::string& interface) {
    auto* conn = getConnectionByInterface(interface);
    if (conn) {
        return conn->link_speed;
    }
    return 0;
}

bool NetworkMonitor::setMonitoringInterval(int interval) {
    if (interval < 1) {
        return false;
    }
    monitoring_interval_ = interval;
    monitor_stats_.monitoring_interval = interval;
    std::cout << "[NetworkMonitor] Monitoring interval set to " << interval << " seconds" << std::endl;
    return true;
}

void NetworkMonitor::registerStateChangeCallback(StateChangeCallback callback) {
    state_callback_ = callback;
}

void NetworkMonitor::registerConnectionCallback(ConnectionCallback callback) {
    connection_callback_ = callback;
}

NetworkConnection* NetworkMonitor::getActiveConnection() {
    for (auto& pair : connections_) {
        if (pair.second.state == NetworkState::CONNECTED) {
            return &pair.second;
        }
    }
    return nullptr;
}

bool NetworkMonitor::simulateStateChange(const std::string& network_id, NetworkState new_state) {
    auto* conn = getConnection(network_id);
    if (!conn) {
        return false;
    }

    std::string old_state = getStateString(conn->state);
    conn->state = new_state;

    // 触发事件
    NetworkEvent event;
    event.type = NetworkEventType::STATE_CHANGED;
    event.network_id = network_id;
    event.old_state = old_state;
    event.new_state = getStateString(new_state);
    event.timestamp = time(nullptr);

    triggerEvent(event);

    // 如果连接状态改变，触发连接回调
    if (new_state == NetworkState::CONNECTED || new_state == NetworkState::DISCONNECTED) {
        if (connection_callback_) {
            connection_callback_(network_id, new_state == NetworkState::CONNECTED);
        }
        monitor_stats_.connection_changes++;
    }

    std::cout << "[NetworkMonitor] Simulated state change: " << network_id 
              << " " << old_state << " -> " << event.new_state << std::endl;

    return true;
}

NetworkMonitor::MonitorStats NetworkMonitor::getMonitorStats() const {
    return monitor_stats_;
}

void NetworkMonitor::readNetworkStatus() {
    // 模拟读取网络状态
    // 实际实现中会读取 /sys/class/net 或使用 netlink socket

    // 添加WiFi连接
    NetworkConnection wifi_conn;
    wifi_conn.network_id = "wifi_network";
    wifi_conn.interface = "wlan0";
    wifi_conn.type = NetworkType::WIFI;
    wifi_conn.state = NetworkState::CONNECTED;
    wifi_conn.signal_strength = 85;
    wifi_conn.ip_address = "192.168.1.100";
    wifi_conn.gateway = "192.168.1.1";
    wifi_conn.link_speed = 300;
    wifi_conn.connected_time = time(nullptr) - 3600;  // 1小时前连接
    connections_[wifi_conn.network_id] = wifi_conn;

    // 添加移动数据连接
    NetworkConnection mobile_conn;
    mobile_conn.network_id = "mobile_network";
    mobile_conn.interface = "rmnet0";
    mobile_conn.type = NetworkType::MOBILE;
    mobile_conn.state = NetworkState::CONNECTED;
    mobile_conn.signal_strength = 60;
    mobile_conn.ip_address = "10.0.0.1";
    mobile_conn.gateway = "10.0.0.1";
    mobile_conn.link_speed = 100;
    mobile_conn.connected_time = time(nullptr) - 7200;  // 2小时前连接
    connections_[mobile_conn.network_id] = mobile_conn;
}

void NetworkMonitor::readNetworkStats() {
    // 模拟读取网络统计信息
    // 实际实现中会读取 /proc/net/dev

    // WiFi统计
    NetworkStats wifi_stats;
    wifi_stats.interface = "wlan0";
    wifi_stats.rx_bytes = 1234567890;
    wifi_stats.tx_bytes = 234567890;
    wifi_stats.rx_packets = 12345678;
    wifi_stats.tx_packets = 2345678;
    wifi_stats.rx_errors = 12;
    wifi_stats.tx_errors = 3;
    wifi_stats.rx_dropped = 45;
    wifi_stats.tx_dropped = 6;
    stats_[wifi_stats.interface] = wifi_stats;

    // 移动数据统计
    NetworkStats mobile_stats;
    mobile_stats.interface = "rmnet0";
    mobile_stats.rx_bytes = 456789012;
    mobile_stats.tx_bytes = 34567890;
    mobile_stats.rx_packets = 4567890;
    mobile_stats.tx_packets = 345678;
    mobile_stats.rx_errors = 23;
    mobile_stats.tx_errors = 7;
    mobile_stats.rx_dropped = 67;
    mobile_stats.tx_dropped = 12;
    stats_[mobile_stats.interface] = mobile_stats;
}

void NetworkMonitor::detectStateChanges() {
    // 在实际实现中，这里会比较之前和当前的网络状态
    // 检测变化并触发相应的事件
}

void NetworkMonitor::triggerEvent(const NetworkEvent& event) {
    monitor_stats_.total_events++;
    
    if (state_callback_) {
        state_callback_(event);
    }

    std::cout << "[NetworkMonitor] Event triggered: " 
              << "Type=" << static_cast<int>(event.type)
              << ", Network=" << event.network_id
              << ", Old=" << event.old_state
              << ", New=" << event.new_state << std::endl;
}

std::string NetworkMonitor::getStateString(NetworkState state) {
    switch (state) {
        case NetworkState::DISCONNECTED: return "DISCONNECTED";
        case NetworkState::CONNECTING: return "CONNECTING";
        case NetworkState::CONNECTED: return "CONNECTED";
        case NetworkState::SUSPENDED: return "SUSPENDED";
        case NetworkState::UNKNOWN: default: return "UNKNOWN";
    }
}

std::string NetworkMonitor::getNetworkTypeString(NetworkType type) {
    switch (type) {
        case NetworkType::WIFI: return "WIFI";
        case NetworkType::MOBILE: return "MOBILE";
        case NetworkType::ETHERNET: return "ETHERNET";
        case NetworkType::BLUETOOTH: return "BLUETOOTH";
        case NetworkType::VPN: return "VPN";
        default: return "UNKNOWN";
    }
}

bool NetworkMonitor::executeCommand(const std::string& command) {
    // 模拟执行命令
    std::cout << "[NetworkMonitor] Executing: " << command << std::endl;
    // 实际实现中会使用 system() 或 popen() 执行命令
    return true;
}