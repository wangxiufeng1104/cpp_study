#include "RouteTableManager.h"
#include <iostream>
#include <sstream>
#include <algorithm>

RouteTableManager::RouteTableManager()
    : initialized_(false), wifi_priority_enabled_(false) {
}

RouteTableManager::~RouteTableManager() {
}

bool RouteTableManager::initialize() {
    if (initialized_) {
        return true;
    }

    // 读取路由表信息
    readRouteTable();

    initialized_ = true;
    return true;
}

std::vector<RouteEntry> RouteTableManager::getRoutes() const {
    return routes_;
}

RouteEntry* RouteTableManager::getDefaultRoute() {
    for (auto& route : routes_) {
        if (route.destination == "0.0.0.0" || route.destination == "default") {
            return &route;
        }
    }
    return nullptr;
}

bool RouteTableManager::addRoute(const RouteEntry& route) {
    // 模拟执行命令: ip route add <destination> via <gateway> dev <interface> metric <metric>
    std::ostringstream cmd;
    cmd << "ip route add " << route.destination;
    if (!route.gateway.empty()) {
        cmd << " via " << route.gateway;
    }
    cmd << " dev " << route.interface;
    cmd << " metric " << route.metric;

    if (executeCommand(cmd.str())) {
        routes_.push_back(route);
        sortRoutes();
        if (route_callback_) {
            route_callback_(route, true);
        }
        return true;
    }

    return false;
}

bool RouteTableManager::deleteRoute(const std::string& destination, const std::string& gateway) {
    // 查找路由条目
    auto it = std::find_if(routes_.begin(), routes_.end(),
        [&destination, &gateway](const RouteEntry& r) {
            return r.destination == destination && r.gateway == gateway;
        });

    if (it == routes_.end()) {
        return false;
    }

    // 模拟执行命令: ip route del <destination>
    std::ostringstream cmd;
    cmd << "ip route del " << destination;
    if (!gateway.empty()) {
        cmd << " via " << gateway;
    }

    if (executeCommand(cmd.str())) {
        if (route_callback_) {
            route_callback_(*it, false);
        }
        routes_.erase(it);
        return true;
    }

    return false;
}

bool RouteTableManager::setDefaultRoute(const std::string& interface, 
                                         const std::string& gateway, int metric) {
    // 先删除现有的默认路由
    auto it = std::find_if(routes_.begin(), routes_.end(),
        [](const RouteEntry& r) {
            return r.destination == "0.0.0.0" || r.destination == "default";
        });

    if (it != routes_.end()) {
        deleteRoute(it->destination, it->gateway);
    }

    // 添加新的默认路由
    RouteEntry new_route;
    new_route.destination = "0.0.0.0/0";
    new_route.gateway = gateway;
    new_route.interface = interface;
    new_route.metric = metric;
    new_route.type = RouteType::UNICAST;
    new_route.protocol = RouteProtocol::STATIC;
    new_route.table_id = 254; // main table

    return addRoute(new_route);
}

bool RouteTableManager::refreshRoutes() {
    routes_.clear();
    readRouteTable();
    return true;
}

std::vector<RouteEntry> RouteTableManager::getRoutesByInterface(const std::string& interface) {
    std::vector<RouteEntry> result;
    for (const auto& route : routes_) {
        if (route.interface == interface) {
            result.push_back(route);
        }
    }
    return result;
}

bool RouteTableManager::enableWifiPriority(const std::string& wifi_interface, 
                                            const std::string& mobile_interface) {
    wifi_interface_ = wifi_interface;
    mobile_interface_ = mobile_interface;

    // 更新路由优先级，WiFi路由的metric值更小，优先级更高
    for (auto& route : routes_) {
        if (route.interface == wifi_interface) {
            route.metric = 100;  // WiFi高优先级
        } else if (route.interface == mobile_interface) {
            route.metric = 200;  // 移动数据低优先级
        }
    }

    // 排序路由表
    sortRoutes();

    wifi_priority_enabled_ = true;

    // 输出路由优先级信息
    std::cout << "[RouteTableManager] WiFi优先策略已启用" << std::endl;
    std::cout << "[RouteTableManager] WiFi接口: " << wifi_interface << " (metric=100)" << std::endl;
    std::cout << "[RouteTableManager] 移动数据接口: " << mobile_interface << " (metric=200)" << std::endl;

    return true;
}

bool RouteTableManager::disableWifiPriority() {
    wifi_priority_enabled_ = false;
    wifi_interface_.clear();
    mobile_interface_.clear();

    std::cout << "[RouteTableManager] WiFi优先策略已禁用" << std::endl;

    return true;
}

void RouteTableManager::registerCallback(RouteCallback callback) {
    route_callback_ = callback;
}

bool RouteTableManager::flushTable(unsigned int table_id) {
    // 模拟执行命令: ip route flush table <table_id>
    std::ostringstream cmd;
    cmd << "ip route flush table " << table_id;

    return executeCommand(cmd.str());
}

void RouteTableManager::readRouteTable() {
    // 模拟读取路由表信息
    // 实际实现中会读取 /proc/net/route 或使用 netlink socket

    // 添加默认路由（WiFi优先）
    RouteEntry default_wifi;
    default_wifi.destination = "0.0.0.0/0";
    default_wifi.gateway = "192.168.1.1";
    default_wifi.interface = "wlan0";
    default_wifi.metric = 100;
    default_wifi.type = RouteType::UNICAST;
    default_wifi.protocol = RouteProtocol::STATIC;
    default_wifi.table_id = 254;
    routes_.push_back(default_wifi);

    // 添加WiFi子网路由
    RouteEntry wifi_subnet;
    wifi_subnet.destination = "192.168.1.0/24";
    wifi_subnet.gateway = "";
    wifi_subnet.interface = "wlan0";
    wifi_subnet.metric = 100;
    wifi_subnet.type = RouteType::UNICAST;
    wifi_subnet.protocol = RouteProtocol::KERNEL;
    wifi_subnet.table_id = 254;
    routes_.push_back(wifi_subnet);

    // 添加移动数据默认路由（作为备用）
    RouteEntry default_mobile;
    default_mobile.destination = "0.0.0.0/0";
    default_mobile.gateway = "10.0.0.1";
    default_mobile.interface = "rmnet0";
    default_mobile.metric = 200;
    default_mobile.type = RouteType::UNICAST;
    default_mobile.protocol = RouteProtocol::STATIC;
    default_mobile.table_id = 254;
    routes_.push_back(default_mobile);

    // 添加移动数据子网路由
    RouteEntry mobile_subnet;
    mobile_subnet.destination = "10.0.0.0/24";
    mobile_subnet.gateway = "";
    mobile_subnet.interface = "rmnet0";
    mobile_subnet.metric = 200;
    mobile_subnet.type = RouteType::UNICAST;
    mobile_subnet.protocol = RouteProtocol::KERNEL;
    mobile_subnet.table_id = 254;
    routes_.push_back(mobile_subnet);

    // 添加本地路由
    RouteEntry local_route;
    local_route.destination = "127.0.0.0/8";
    local_route.gateway = "";
    local_route.interface = "lo";
    local_route.metric = 0;
    local_route.type = RouteType::LOCAL;
    local_route.protocol = RouteProtocol::KERNEL;
    local_route.table_id = 255;
    routes_.push_back(local_route);

    // 排序路由表
    sortRoutes();
}

bool RouteTableManager::executeCommand(const std::string& command) {
    // 模拟执行命令
    std::cout << "[RouteTableManager] Executing: " << command << std::endl;
    // 实际实现中会使用 system() 或 popen() 执行命令
    return true;
}

bool RouteTableManager::updateRoutePriorities() {
    if (!wifi_priority_enabled_) {
        return true;
    }

    // 根据WiFi优先策略更新路由优先级
    for (auto& route : routes_) {
        if (route.interface == wifi_interface_) {
            route.metric = 100;
        } else if (route.interface == mobile_interface_) {
            route.metric = 200;
        }
    }

    sortRoutes();
    return true;
}

void RouteTableManager::sortRoutes() {
    // 按metric值排序，metric值越小优先级越高
    std::sort(routes_.begin(), routes_.end(),
        [](const RouteEntry& a, const RouteEntry& b) {
            return a.metric < b.metric;
        });
}