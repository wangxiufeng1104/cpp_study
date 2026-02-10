# NetDaemon - Android网络守护进程核心功能实现

## 项目简介

本项目使用C++实现了Android系统中netd守护进程的6个核心功能模块。netd是Android系统中的网络管理守护进程，负责管理和配置系统网络资源。

## 核心功能模块

### 1. 网络接口管理器 (NetworkInterfaceManager)

**功能：**
- 配置和管理网络接口（WiFi、移动数据、以太网等）
- 接口状态监控和更新
- 网络接口的启用和禁用
- 设置接口metric值（用于路由优先级）
- IP地址配置

**主要方法：**
- `getInterfaces()` - 获取所有网络接口
- `bringUp()` / `bringDown()` - 启用/禁用接口
- `setIPAddress()` - 配置IP地址
- `setMetric()` - 设置路由优先级

### 2. 路由表管理器 (RouteTableManager)

**功能：**
- 维护系统路由表
- 根据网络类型设置不同路由规则
- 实现多网络环境下的路由选择
- **WiFi优先上网**的核心实现

**主要方法：**
- `getRoutes()` - 获取所有路由条目
- `setDefaultRoute()` - 设置默认路由
- `enableWifiPriority()` - 启用WiFi优先策略
- `addRoute()` / `deleteRoute()` - 管理路由条目

**WiFi优先实现原理：**
- 为不同网络接口设置不同的路由metric值
- WiFi路由的metric值更小（100），优先级更高
- 移动数据路由的metric值更大（200），优先级更低
- 系统自动选择metric值最小的路由作为默认路由

### 3. 防火墙和流量控制管理器 (FirewallManager)

**功能：**
- 管理iptables规则
- 实现数据包过滤和转发
- 支持流量限制和QoS
- NAT配置和端口转发

**主要方法：**
- `addFirewallRule()` - 添加防火墙规则
- `addTrafficControlRule()` - 添加流量控制规则
- `enableNAT()` / `disableNAT()` - NAT配置
- `portForward()` - 端口转发

### 4. DNS管理器 (DNSManager)

**功能：**
- 配置DNS服务器
- DNS解析和缓存
- 管理DNS解析策略
- 系统DNS配置

**主要方法：**
- `resolve()` - 域名解析
- `addDNSServer()` - 添加DNS服务器
- `clearCache()` - 清空DNS缓存
- `configureSystemDNS()` - 配置系统DNS

**特性：**
- 支持DNS查询类型（A, AAAA, MX, CNAME等）
- DNS缓存机制，提高解析效率
- 统计信息跟踪（查询数、缓存命中率等）

### 5. 网络策略管理器 (NetworkPolicyManager)

**功能：**
- 根据ConnectivityService的策略执行网络配置
- 实现网络优先级规则
- 处理网络切换逻辑
- 应用网络使用策略

**主要方法：**
- `setNetworkPriority()` - 设置网络优先级
- `setNetworkMetered()` - 设置计量网络属性
- `applyWifiPriorityPolicy()` - 应用WiFi优先策略
- `handleNetworkSwitch()` - 处理网络切换

**网络策略类型：**
- ALLOWED - 允许使用
- RESTRICTED - 受限使用
- BLOCKED - 阻止使用

### 6. 网络状态监控器 (NetworkMonitor)

**功能：**
- 实时监控网络连接状态
- 检测网络变化
- 通知上层网络状态变化
- 网络连通性检查

**主要方法：**
- `getConnections()` - 获取所有网络连接
- `getNetworkStats()` - 获取网络统计信息
- `checkConnectivity()` - 检查连通性
- `simulateStateChange()` - 模拟状态变化（测试用）

**监控信息：**
- 连接状态、信号强度、IP地址
- 网关、链路速度
- 接收/发送字节数和包数
- 错误和丢包统计

## 项目结构

```
NetDaemon/
├── include/                    # 头文件目录
│   ├── NetworkInterfaceManager.h
│   ├── RouteTableManager.h
│   ├── FirewallManager.h
│   ├── DNSManager.h
│   ├── NetworkPolicyManager.h
│   └── NetworkMonitor.h
├── src/                       # 源文件目录
│   ├── NetworkInterfaceManager.cpp
│   ├── RouteTableManager.cpp
│   ├── FirewallManager.cpp
│   ├── DNSManager.cpp
│   ├── NetworkPolicyManager.cpp
│   ├── NetworkMonitor.cpp
│   └── main.cpp
├── build/                     # 构建目录
├── CMakeLists.txt             # CMake构建配置
└── README.md                  # 项目说明文档
```

## 编译和运行

### 编译

```bash
cd NetDaemon
mkdir build
cd build
cmake ..
make
```

### 运行

```bash
./NetDaemon
```

## 技术特点

1. **模块化设计** - 每个功能模块独立，职责清晰
2. **回调机制** - 支持事件回调，便于状态通知
3. **模拟实现** - 使用模拟数据演示功能，适合学习和测试
4. **C++11标准** - 使用现代C++特性
5. **CMake构建** - 跨平台构建系统

## 实际应用说明

本实现是针对Android netd守护进程核心功能的教学演示。在实际的Android系统中：

1. 命令执行会使用真实的系统命令（ip, iptables, tc等）
2. 网络状态通过netlink socket实时获取
3. 与ConnectivityService通过Binder进行通信
4. 需要root权限执行网络配置命令

## 许可证

本项目仅供学习和参考使用。

## 作者

NetDaemon Project Team