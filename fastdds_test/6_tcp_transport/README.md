# TCP 传输示例 - FastDDS TCP 协议配置

本示例演示如何配置 FastDDS 使用 TCP 作为传输协议。

## 概念说明

### 为什么使用 TCP?

FastDDS 默认使用 UDP 作为传输协议，但在某些场景下需要使用 TCP：

- **防火墙穿越**: TCP 更容易穿透防火墙
- **广域网通信**: TCP 在广域网上更稳定
- **可靠性要求**: TCP 提供天然的可靠传输
- **NAT 穿透**: TCP 更容易实现 NAT 穿透

### UDP vs TCP

| 特性 | UDP | TCP |
|------|-----|-----|
| 连接方式 | 无连接 | 面向连接 |
| 可靠性 | 不保证 | 保证 |
| 延迟 | 低 | 较高 |
| 广播/多播 | 支持 | 不支持 |
| 防火墙 | 可能被阻止 | 通常允许 |

## 文件结构

```
6_tcp_transport/
├── CMakeLists.txt      # 构建配置
├── publisher.cpp       # TCP 发布者实现
├── subscriber.cpp      # TCP 订阅者实现
└── README.md           # 本文件
```

## TCP 配置详解

### 发布者 (服务端) 配置

```cpp
DomainParticipantQos part_qos;
part_qos.name("TCP_Publisher");

// 禁用默认传输
part_qos.transport().use_builtin_transports = false;

// 创建 TCP 传输描述符
auto tcp_descriptor = std::make_shared<TCPv4TransportDescriptor>();
tcp_descriptor->sendBufferSize = 0;     // 使用默认值
tcp_descriptor->receiveBufferSize = 0;  // 使用默认值

// 设置 TCP 监听端口
tcp_descriptor->add_listener_port(port);

// 添加到传输配置
part_qos.transport().user_transports.push_back(tcp_descriptor);
```

### 订阅者 (客户端) 配置

```cpp
DomainParticipantQos part_qos;
part_qos.name("TCP_Subscriber");

// 禁用默认传输
part_qos.transport().use_builtin_transports = false;

// 创建 TCP 传输描述符
auto tcp_descriptor = std::make_shared<TCPv4TransportDescriptor>();

// 配置连接到远程服务器
// 通过 initialPeersList 配置初始对等点
eprosima::fastrtps::rtps::Locator_t locator;
locator.kind = LOCATOR_KIND_TCPv4;
locator.port = port;
// 设置 IP 地址...

part_qos.wire_protocol().builtin.initialPeersList.push_back(locator);
part_qos.transport().user_transports.push_back(tcp_descriptor);
```

## 编译

在项目根目录执行:

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## 运行

### 同一主机测试

#### 终端 1 - 启动发布者 (监听端口 5100)

```bash
./6_tcp_transport_publisher 5100
```

#### 终端 2 - 启动订阅者

```bash
./6_tcp_transport_subscriber 127.0.0.1 5100
```

### 跨主机测试

#### 主机 A (服务端) - 启动发布者

```bash
./6_tcp_transport_publisher 5100
```

#### 主机 B (客户端) - 启动订阅者

```bash
./6_tcp_transport_subscriber <服务器IP> 5100
```

## 预期输出

### 发布者输出

```
========================================
TCP传输 发布者示例
========================================
TCP发布者初始化成功!
TCP监听端口: 5100

等待TCP订阅者连接...
TCP匹配到订阅者 (匹配数: 1)
[2026-03-16 16:10:00] 已发布(TCP): TCP Message #0
[2026-03-16 16:10:01] 已发布(TCP): TCP Message #1
...
```

### 订阅者输出

```
========================================
TCP传输 订阅者示例
========================================
TCP订阅者初始化成功!
连接到: 127.0.0.1:5100

TCP匹配到发布者 (匹配数: 1)
[2026-03-16 16:10:00] 收到(TCP): TCP Message #0
[2026-03-16 16:10:01] 收到(TCP): TCP Message #1
...
```

## 关键代码说明

### TCPv4TransportDescriptor 主要配置项

```cpp
auto tcp_descriptor = std::make_shared<TCPv4TransportDescriptor>();

// 缓冲区大小 (0 = 使用系统默认)
tcp_descriptor->sendBufferSize = 0;
tcp_descriptor->receiveBufferSize = 0;

// 添加监听端口 (服务端)
tcp_descriptor->add_listener_port(5100);

// 设置最大传输单元
tcp_descriptor->interfaceWhiteList = {"127.0.0.1"};  // 限制接口

// TCP 特定选项
tcp_descriptor->keep_alive_frequency_ms = 5000;  // 心跳间隔
tcp_descriptor->keep_alive_timeout_ms = 15000;   // 超时时间
```

### 完整的发布者初始化

```cpp
bool init(uint16_t port)
{
    DomainParticipantQos part_qos;
    part_qos.name("TCP_Publisher");
    part_qos.transport().use_builtin_transports = false;

    auto tcp_descriptor = std::make_shared<TCPv4TransportDescriptor>();
    tcp_descriptor->add_listener_port(port);
    part_qos.transport().user_transports.push_back(tcp_descriptor);

    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, part_qos);
    if (!participant_) return false;

    // ... 后续 Topic, Publisher, DataWriter 创建
}
```

## 高级配置

### TCP with TLS (安全传输)

```cpp
auto tcp_descriptor = std::make_shared<TCPv4TransportDescriptor>();

// 配置 TLS
tcp_descriptor->tls = std::make_shared<TCPTransportDescriptor::TLSConfig>();
tcp_descriptor->tls->options = TCPTransportDescriptor::TLSConfig::TLS_OPTIONS::DEFAULT;
tcp_descriptor->tls->password = "password";
tcp_descriptor->tls->cert_chain_file = "cert.pem";
tcp_descriptor->tls->private_key_file = "key.pem";
tcp_descriptor->tls->tmp_dh_file = "dh.pem";
```

### 多网卡绑定

```cpp
// 限制使用特定网卡
tcp_descriptor->interfaceWhiteList = {"192.168.1.100"};
```

## 防火墙配置

### Linux (iptables)

```bash
# 开放 TCP 端口
sudo iptables -A INPUT -p tcp --dport 5100 -j ACCEPT
```

### Linux (firewalld)

```bash
# 开放 TCP 端口
sudo firewall-cmd --add-port=5100/tcp --permanent
sudo firewall-cmd --reload
```

## 应用场景

| 场景 | 说明 |
|------|------|
| 云服务通信 | 云环境通常限制 UDP |
| 跨数据中心 | 广域网更稳定 |
| 企业网络 | 穿越企业防火墙 |
| 移动网络 | 某些运营商限制 UDP |

## 注意事项

1. **端口开放**: 确保服务器端口在防火墙中开放
2. **延迟**: TCP 延迟比 UDP 高
3. **连接管理**: 需要处理连接断开和重连
4. **地址配置**: 跨主机时使用正确的 IP 地址

## 故障排查

### 连接失败

1. 检查防火墙配置
2. 确认 IP 地址和端口正确
3. 验证网络连通性 (`ping`, `telnet`)
4. 检查 FastDDS 日志

### 日志启用

```cpp
// 启用详细日志
#include <fastdds/dds/log/Log.hpp>
using namespace eprosima::fastdds::dds;

Log::SetVerbosity(Log::Kind::Info);
Log::SetCategoryFilter(std::regex("TCP"));
```

## 下一步

- 查看 `7_monitoring` 了解实时监控功能
- 查看 `9_security` 了解安全加密通信