#include "NetworkInterfaceManager.h"
#include "RouteTableManager.h"
#include "FirewallManager.h"
#include "DNSManager.h"
#include "NetworkPolicyManager.h"
#include "NetworkMonitor.h"
#include <iostream>
#include <thread>
#include <chrono>

/**
 * @brief NetDaemon - Android网络守护进程模拟
 * 
 * 这个程序演示了Android系统中netd守护进程的6个核心功能：
 * 1. 网络接口管理
 * 2. 路由表管理
 * 3. 防火墙和流量控制
 * 4. DNS管理
 * 5. 网络策略实施
 * 6. 网络状态监控
 */

void printSeparator() {
    std::cout << "\n" << std::string(80, '=') << "\n" << std::endl;
}

void printHeader(const std::string& title) {
    printSeparator();
    std::cout << "  " << title << std::endl;
    printSeparator();
}

void demoNetworkInterfaceManager(NetworkInterfaceManager& if_mgr) {
    printHeader("1. 网络接口管理器演示 (NetworkInterfaceManager)");
    
    // 初始化
    if_mgr.initialize();
    
    // 获取所有接口
    auto interfaces = if_mgr.getInterfaces();
    std::cout << "网络接口列表:\n" << std::endl;
    for (const auto& iface : interfaces) {
        std::cout << "  接口名: " << iface.name << std::endl;
        std::cout << "  IP地址: " << iface.ip_address << "/" << iface.netmask << std::endl;
        std::cout << "  MAC地址: " << iface.mac_address << std::endl;
        std::cout << "  状态: " << (iface.state == InterfaceState::UP ? "UP" : "DOWN") << std::endl;
        std::cout << "  类型: " << (iface.type == InterfaceType::WIFI ? "WiFi" : 
                                   iface.type == InterfaceType::MOBILE_DATA ? "移动数据" : "其他") << std::endl;
        std::cout << "  Metric: " << iface.metric << std::endl;
        std::cout << std::endl;
    }

    // 设置接口metric（WiFi优先）
    std::cout << "设置WiFi接口优先级..." << std::endl;
    if_mgr.setMetric("wlan0", 100);
    if_mgr.setMetric("rmnet0", 200);
}

void demoRouteTableManager(RouteTableManager& route_mgr) {
    printHeader("2. 路由表管理器演示 (RouteTableManager)");
    
    // 初始化
    route_mgr.initialize();
    
    // 获取所有路由
    auto routes = route_mgr.getRoutes();
    std::cout << "路由表:\n" << std::endl;
    for (const auto& route : routes) {
        std::cout << "  目标: " << route.destination << std::endl;
        std::cout << "  网关: " << (route.gateway.empty() ? "-" : route.gateway) << std::endl;
        std::cout << "  接口: " << route.interface << std::endl;
        std::cout << "  Metric: " << route.metric << std::endl;
        std::cout << std::endl;
    }

    // 启用WiFi优先
    std::cout << "启用WiFi优先路由策略..." << std::endl;
    route_mgr.enableWifiPriority("wlan0", "rmnet0");
}

void demoFirewallManager(FirewallManager& fw_mgr) {
    printHeader("3. 防火墙管理器演示 (FirewallManager)");
    
    // 初始化
    fw_mgr.initialize();
    
    // 添加防火墙规则
    std::cout << "添加防火墙规则..." << std::endl;
    FirewallRule rule1;
    rule1.table = IptablesTable::FILTER;
    rule1.chain = IptablesChain::INPUT;
    rule1.protocol = "tcp";
    rule1.src_port = "80";
    rule1.action = "ACCEPT";
    fw_mgr.addFirewallRule(rule1);

    // 添加流量控制规则
    std::cout << "\n添加流量控制规则..." << std::endl;
    TrafficControlRule tc_rule;
    tc_rule.interface = "rmnet0";
    tc_rule.rate = 1000;  // 1 Mbps
    tc_rule.latency = 50;  // 50ms
    tc_rule.loss = 1;  // 1%
    fw_mgr.addTrafficControlRule(tc_rule);

    // 显示防火墙规则
    auto rules = fw_mgr.getFirewallRules();
    std::cout << "\n防火墙规则数量: " << rules.size() << std::endl;
}

void demoDNSManager(DNSManager& dns_mgr) {
    printHeader("4. DNS管理器演示 (DNSManager)");
    
    // 初始化
    dns_mgr.initialize();
    
    // 显示DNS服务器
    auto servers = dns_mgr.getDNSServers();
    std::cout << "DNS服务器:\n" << std::endl;
    for (const auto& server : servers) {
        std::cout << "  " << server.ip_address << " (优先级: " << server.priority << ")" << std::endl;
    }

    // DNS解析
    std::cout << "\n执行DNS解析..." << std::endl;
    auto result = dns_mgr.resolve("www.example.com", DNSQueryType::A);
    if (result.success) {
        std::cout << "  域名: " << result.hostname << std::endl;
        std::cout << "  IP地址: ";
        for (size_t i = 0; i < result.ip_addresses.size(); ++i) {
            std::cout << result.ip_addresses[i];
            if (i < result.ip_addresses.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }

    // 显示统计信息
    auto stats = dns_mgr.getStats();
    std::cout << "\nDNS统计:\n" << std::endl;
    std::cout << "  总查询数: " << stats.total_queries << std::endl;
    std::cout << "  缓存命中: " << stats.cache_hits << std::endl;
    std::cout << "  缓存未命中: " << stats.cache_misses << std::endl;
}

void demoNetworkPolicyManager(NetworkPolicyManager& policy_mgr) {
    printHeader("5. 网络策略管理器演示 (NetworkPolicyManager)");
    
    // 初始化
    policy_mgr.initialize();
    
    // 显示网络策略
    auto policies = policy_mgr.getNetworkPolicies();
    std::cout << "网络策略:\n" << std::endl;
    for (const auto& policy : policies) {
        std::cout << "  网络: " << policy.network_id << std::endl;
        std::cout << "  优先级: " << (policy.priority == NetworkPriority::HIGH ? "HIGH" : "LOW") << std::endl;
        std::cout << "  计量: " << (policy.metered ? "是" : "否") << std::endl;
        std::cout << "  带宽限制: " << policy.bandwidth_limit << " kbps" << std::endl;
        std::cout << std::endl;
    }

    // 应用WiFi优先策略
    std::cout << "应用WiFi优先策略..." << std::endl;
    policy_mgr.applyWifiPriorityPolicy("wifi_network", "mobile_network");
    
    // 设置活动网络
    policy_mgr.setActiveNetwork("wifi_network");
    std::cout << "当前活动网络: " << policy_mgr.getActiveNetwork() << std::endl;
}

void demoNetworkMonitor(NetworkMonitor& monitor) {
    printHeader("6. 网络监控器演示 (NetworkMonitor)");
    
    // 初始化
    monitor.initialize();
    monitor.start();
    
    // 获取连接
    auto connections = monitor.getConnections();
    std::cout << "网络连接:\n" << std::endl;
    for (const auto& conn : connections) {
        std::cout << "  网络: " << conn.network_id << std::endl;
        std::cout << "  接口: " << conn.interface << std::endl;
        std::cout << "  类型: " << (conn.type == NetworkType::WIFI ? "WiFi" : "移动数据") << std::endl;
        std::cout << "  状态: " << (conn.state == NetworkState::CONNECTED ? "已连接" : "未连接") << std::endl;
        std::cout << "  信号强度: " << conn.signal_strength << "%" << std::endl;
        std::cout << "  IP地址: " << conn.ip_address << std::endl;
        std::cout << "  网关: " << conn.gateway << std::endl;
        std::cout << "  链路速度: " << conn.link_speed << " Mbps" << std::endl;
        std::cout << std::endl;
    }

    // 获取网络统计
    std::cout << "网络统计 (wlan0):\n" << std::endl;
    auto stats = monitor.getNetworkStats("wlan0");
    std::cout << "  接收字节: " << stats.rx_bytes << std::endl;
    std::cout << "  发送字节: " << stats.tx_bytes << std::endl;
    std::cout << "  接收包数: " << stats.rx_packets << std::endl;
    std::cout << "  发送包数: " << stats.tx_packets << std::endl;

    // 检查连通性
    std::cout << "\n检查网络连通性..." << std::endl;
    bool connected = monitor.checkConnectivity("8.8.8.8");
    std::cout << "  连通状态: " << (connected ? "正常" : "异常") << std::endl;

    // 模拟状态变化
    std::cout << "\n模拟WiFi断开..." << std::endl;
    monitor.simulateStateChange("wifi_network", NetworkState::DISCONNECTED);
    
    // 显示监控器统计
    auto monitor_stats = monitor.getMonitorStats();
    std::cout << "\n监控器统计:\n" << std::endl;
    std::cout << "  运行中: " << (monitor_stats.running ? "是" : "否") << std::endl;
    std::cout << "  总事件数: " << monitor_stats.total_events << std::endl;
    std::cout << "  连接变化: " << monitor_stats.connection_changes << std::endl;
}

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                   NetDaemon - 网络守护进程                     ║" << std::endl;
    std::cout << "║                  Android netd 核心功能演示                     ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════════╝" << std::endl;

    // 创建各个管理器实例
    NetworkInterfaceManager if_mgr;
    RouteTableManager route_mgr;
    FirewallManager fw_mgr;
    DNSManager dns_mgr;
    NetworkPolicyManager policy_mgr;
    NetworkMonitor monitor;

    // 注册回调（可选）
    monitor.registerStateChangeCallback([](const NetworkEvent& event) {
        std::cout << "[回调] 网络事件: " << event.network_id 
                  << " -> " << event.new_state << std::endl;
    });

    // 演示各个功能模块
    demoNetworkInterfaceManager(if_mgr);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    demoRouteTableManager(route_mgr);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    demoFirewallManager(fw_mgr);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    demoDNSManager(dns_mgr);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    demoNetworkPolicyManager(policy_mgr);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    demoNetworkMonitor(monitor);

    // 停止监控
    monitor.stop();

    printSeparator();
    std::cout << "  NetDaemon 演示完成！" << std::endl;
    std::cout << "  所有核心功能模块已成功初始化和运行。" << std::endl;
    printSeparator();

    return 0;
}