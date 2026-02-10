#include "NetworkInterfaceManager.h"
#include <iostream>
#include <sstream>

NetworkInterfaceManager::NetworkInterfaceManager()
    : initialized_(false) {
}

NetworkInterfaceManager::~NetworkInterfaceManager() {
}

bool NetworkInterfaceManager::initialize() {
    if (initialized_) {
        return true;
    }

    // 读取网络接口信息
    readInterfaceInfo();

    initialized_ = true;
    return true;
}

std::vector<NetworkInterface> NetworkInterfaceManager::getInterfaces() const {
    std::vector<NetworkInterface> result;
    for (const auto& pair : interfaces_) {
        result.push_back(pair.second);
    }
    return result;
}

NetworkInterface* NetworkInterfaceManager::getInterface(const std::string& name) {
    auto it = interfaces_.find(name);
    if (it != interfaces_.end()) {
        return &it->second;
    }
    return nullptr;
}

bool NetworkInterfaceManager::bringUp(const std::string& name) {
    auto* iface = getInterface(name);
    if (!iface) {
        std::cerr << "Interface " << name << " not found" << std::endl;
        return false;
    }

    // 模拟执行命令: ip link set <name> up
    std::ostringstream cmd;
    cmd << "ip link set " << name << " up";
    
    if (executeCommand(cmd.str())) {
        iface->state = InterfaceState::UP;
        if (state_callback_) {
            state_callback_(*iface);
        }
        return true;
    }

    return false;
}

bool NetworkInterfaceManager::bringDown(const std::string& name) {
    auto* iface = getInterface(name);
    if (!iface) {
        std::cerr << "Interface " << name << " not found" << std::endl;
        return false;
    }

    // 模拟执行命令: ip link set <name> down
    std::ostringstream cmd;
    cmd << "ip link set " << name << " down";
    
    if (executeCommand(cmd.str())) {
        iface->state = InterfaceState::DOWN;
        if (state_callback_) {
            state_callback_(*iface);
        }
        return true;
    }

    return false;
}

bool NetworkInterfaceManager::setIPAddress(const std::string& name, 
                                            const std::string& ip_address, 
                                            const std::string& netmask) {
    auto* iface = getInterface(name);
    if (!iface) {
        std::cerr << "Interface " << name << " not found" << std::endl;
        return false;
    }

    // 模拟执行命令: ip addr add <ip>/<netmask> dev <name>
    std::ostringstream cmd;
    cmd << "ip addr add " << ip_address << "/" << netmask << " dev " << name;
    
    if (executeCommand(cmd.str())) {
        iface->ip_address = ip_address;
        iface->netmask = netmask;
        if (state_callback_) {
            state_callback_(*iface);
        }
        return true;
    }

    return false;
}

bool NetworkInterfaceManager::setMetric(const std::string& name, int metric) {
    auto* iface = getInterface(name);
    if (!iface) {
        std::cerr << "Interface " << name << " not found" << std::endl;
        return false;
    }

    // 模拟执行命令: ip link set <name> metric <metric>
    std::ostringstream cmd;
    cmd << "ip link set " << name << " metric " << metric;
    
    if (executeCommand(cmd.str())) {
        iface->metric = metric;
        if (state_callback_) {
            state_callback_(*iface);
        }
        return true;
    }

    return false;
}

bool NetworkInterfaceManager::refreshInterfaces() {
    readInterfaceInfo();
    return true;
}

void NetworkInterfaceManager::registerCallback(InterfaceCallback callback) {
    state_callback_ = callback;
}

void NetworkInterfaceManager::readInterfaceInfo() {
    // 模拟读取网络接口信息
    // 实际实现中会读取 /proc/net/dev 或使用 netlink socket
    
    // 清空现有接口
    interfaces_.clear();

    // 添加模拟的网络接口
    NetworkInterface wifi_iface;
    wifi_iface.name = "wlan0";
    wifi_iface.ip_address = "192.168.1.100";
    wifi_iface.netmask = "24";
    wifi_iface.mac_address = "00:11:22:33:44:55";
    wifi_iface.state = InterfaceState::UP;
    wifi_iface.type = InterfaceType::WIFI;
    wifi_iface.metric = 100;  // WiFi优先级高，metric值小
    interfaces_[wifi_iface.name] = wifi_iface;

    NetworkInterface mobile_iface;
    mobile_iface.name = "rmnet0";
    mobile_iface.ip_address = "10.0.0.1";
    mobile_iface.netmask = "24";
    mobile_iface.mac_address = "00:11:22:33:44:66";
    mobile_iface.state = InterfaceState::UP;
    mobile_iface.type = InterfaceType::MOBILE_DATA;
    mobile_iface.metric = 200;  // 移动数据优先级低，metric值大
    interfaces_[mobile_iface.name] = mobile_iface;

    NetworkInterface loopback_iface;
    loopback_iface.name = "lo";
    loopback_iface.ip_address = "127.0.0.1";
    loopback_iface.netmask = "8";
    loopback_iface.mac_address = "00:00:00:00:00:00";
    loopback_iface.state = InterfaceState::UP;
    loopback_iface.type = InterfaceType::LOOPBACK;
    loopback_iface.metric = 0;
    interfaces_[loopback_iface.name] = loopback_iface;
}

bool NetworkInterfaceManager::executeCommand(const std::string& command) {
    // 模拟执行命令
    std::cout << "[NetworkInterfaceManager] Executing: " << command << std::endl;
    // 实际实现中会使用 system() 或 popen() 执行命令
    return true;
}