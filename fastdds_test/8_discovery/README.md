# 发现机制示例 - FastDDS 参与者发现

本示例演示 FastDDS 的参与者发现机制，包括 Simple Discovery 和配置选项。

## 概念说明

### DDS 发现机制

DDS 的发现机制允许 DomainParticipant 自动发现同一 Domain 中的其他参与者，无需显式配置。

### 发现过程

```
参与者启动
    │
    ▼
发送 SPDP 消息 (Simple Participant Discovery Protocol)
    │
    ▼
其他参与者接收 SPDP 消息
    │
    ▼
建立参与者间的元数据连接
    │
    ▼
SEDP 消息交换 (Simple Endpoint Discovery Protocol)
    │
    ▼
匹配 DataReader/DataWriter
    │
    ▼
开始数据通信
```

### 发现协议

| 协议 | 说明 |
|------|------|
| **SPDP** | Simple Participant Discovery Protocol - 参与者发现 |
| **SEDP** | Simple Endpoint Discovery Protocol - 端点发现 |

## 文件结构

```
8_discovery/
├── CMakeLists.txt           # 构建配置
├── discovery_participant.cpp # 发现参与者实现
└── README.md                # 本文件
```

## 发现配置选项

### 1. Domain ID

同一 Domain ID 的参与者才能相互发现：

```cpp
DomainParticipantQos qos;
// Domain ID 0 - 默认值
participant = factory->create_participant(0, qos);
```

### 2. 参与者名称

```cpp
DomainParticipantQos qos;
qos.name("MyParticipant");
```

### 3. 发现配置

```cpp
DomainParticipantQos qos;

// 简单发现配置
qos.wire_protocol().builtin.discovery_config.discoveryServersList = {};

// 发现租约时间
qos.wire_protocol().builtin.discovery_config.leaseDuration = Duration_t(60, 0);
qos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = Duration_t(10, 0);
```

### 4. 初始对等点

```cpp
// 配置初始对等点 (用于 TCP 或受限网络)
eprosima::fastrtps::rtps::Locator_t locator;
locator.kind = LOCATOR_KIND_UDPv4;
locator.port = 7400;
// 设置 IP 地址...

qos.wire_protocol().builtin.initialPeersList.push_back(locator);
```

## 编译

在项目根目录执行:

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## 运行

### 终端 1 - 启动参与者 A

```bash
./8_discovery_participant A
```

### 终端 2 - 启动参与者 B

```bash
./8_discovery_participant B
```

## 预期输出

### 参与者 A 输出

```
========================================
发现机制示例 - 参与者 A
========================================
参与者初始化成功!
参与者名称: Participant_A
Domain ID: 0
本地 GUID: 01.0f.01.01.00.00.00.00.00.00.00.01

等待发现其他参与者...

[2026-03-16 16:20:00] 发现参与者!
  名称: Participant_B
  GUID: 01.0f.01.01.00.00.00.00.00.00.00.02
  状态: ALIVE

发现总结:
  已发现参与者数: 1

按 Ctrl+C 退出...
```

### 参与者 B 输出

```
========================================
发现机制示例 - 参与者 B
========================================
参与者初始化成功!
参与者名称: Participant_B
Domain ID: 0
本地 GUID: 01.0f.01.01.00.00.00.00.00.00.00.02

等待发现其他参与者...

[2026-03-16 16:20:00] 发现参与者!
  名称: Participant_A
  GUID: 01.0f.01.01.00.00.00.00.00.00.00.01
  状态: ALIVE

发现总结:
  已发现参与者数: 1

按 Ctrl+C 退出...
```

## 关键代码说明

### 参与者发现监听器

```cpp
class ParticipantListener : public DomainParticipantListener
{
public:
    void on_participant_discovery(DomainParticipant* participant,
                                   ParticipantDiscoveryInfo&& info) override
    {
        switch (info.status)
        {
            case ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT:
                std::cout << "\n发现参与者!" << std::endl;
                std::cout << "  名称: " << info.info.name() << std::endl;
                std::cout << "  GUID: " << info.info.guid() << std::endl;
                std::cout << "  状态: ALIVE" << std::endl;
                break;
                
            case ParticipantDiscoveryInfo::REMOVED_PARTICIPANT:
            case ParticipantDiscoveryInfo::DROPPED_PARTICIPANT:
                std::cout << "\n参与者离开!" << std::endl;
                std::cout << "  名称: " << info.info.name() << std::endl;
                std::cout << "  状态: REMOVED/DROPPED" << std::endl;
                break;
        }
    }
};
```

### 获取本地参与者信息

```cpp
void print_local_info(DomainParticipant* participant)
{
    std::cout << "参与者名称: " << participant->get_qos().name() << std::endl;
    std::cout << "Domain ID: " << participant->get_domain_id() << std::endl;
    std::cout << "本地 GUID: " << participant->guid() << std::endl;
}
```

### 查询已发现的参与者

```cpp
// 获取所有已发现的参与者
std::vector<ParticipantInfo> discovered;
participant->get_discovered_participants(discovered);

for (const auto& info : discovered)
{
    std::cout << "参与者: " << info.name() << std::endl;
    std::cout << "GUID: " << info.guid() << std::endl;
}
```

## 发现模式

### Simple Discovery (默认)

```cpp
DomainParticipantQos qos;

// 简单发现 - 自动组播发现
qos.wire_protocol().builtin.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = true;
qos.wire_protocol().builtin.discovery_config.use_SIMPLE_ParticipantDiscoveryProtocol = true;

// 发现初始公告
qos.wire_protocol().builtin.discovery_config.simple.initialAnnouncements = 5;
qos.wire_protocol().builtin.discovery_config.simple.initialAnnouncementPeriod = Duration_t(0, 100000000);  // 100ms
```

### Static Discovery (静态发现)

预先配置已知参与者的信息：

```cpp
DomainParticipantQos qos;

// 禁用动态发现
qos.wire_protocol().builtin.discovery_config.use_SIMPLE_ParticipantDiscoveryProtocol = false;

// 静态配置远程参与者
RemoteParticipantAttributes remote;
remote.guid = GuidPrefix_t(...);
remote.metatraffic_locators.add_udp_locator(7400);
qos.wire_protocol().builtin.discovery_config.static_discovery.participants.push_back(remote);
```

### Server-Client Discovery (发现服务器)

集中式发现服务器模式：

```cpp
// 服务器配置
DomainParticipantQos server_qos;
server_qos.wire_protocol().builtin.discovery_config.discoveryServersList = {};

// 客户端配置
DomainParticipantQos client_qos;
Locator_t server_locator;
server_locator.kind = LOCATOR_KIND_UDPv4;
server_locator.port = 7400;
// 设置 IP...

DiscoveryServer server;
server.guidPrefix = GuidPrefix_t("SERVER");
server.metatraffic_locator = server_locator;

client_qos.wire_protocol().builtin.discovery_config.discoveryServersList.push_back(server);
```

## 发现时间参数

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `leaseDuration` | 租约超时时间 | 300s |
| `leaseDuration_announcementperiod` | 公告周期 | 30s |
| `initialAnnouncements` | 初始公告次数 | 5 |
| `initialAnnouncementPeriod` | 初始公告间隔 | 100ms |

## 故障排查

### 参与者无法发现

1. **检查 Domain ID**: 确保在同一 Domain
2. **检查网络**: 组播是否被阻止
3. **检查防火墙**: 开放相关端口
4. **查看日志**: 启用发现日志

```cpp
// 启用发现日志
#include <fastdds/dds/log/Log.hpp>
Log::SetVerbosity(Log::Kind::Info);
Log::SetCategoryFilter(std::regex("DISCOVERY"));
```

### 发现延迟高

调整发现参数：

```cpp
// 加快发现速度
qos.wire_protocol().builtin.discovery_config.leaseDuration = Duration_t(30, 0);
qos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = Duration_t(5, 0);
qos.wire_protocol().builtin.discovery_config.simple.initialAnnouncements = 10;
```

## 应用场景

| 场景 | 发现模式 |
|------|----------|
| 局域网自动化 | Simple Discovery |
| 受限网络 | Static Discovery |
| 大规模系统 | Server-Client Discovery |
| 跨网络 | Initial Peers |

## 注意事项

1. **租约时间**: 设置合理的租约时间，避免误判节点故障
2. **组播依赖**: Simple Discovery 依赖组播，某些网络可能不支持
3. **发现延迟**: 系统启动时需要考虑发现时间
4. **资源消耗**: 大量参与者时，发现会增加网络和 CPU 负载

## 下一步

- 查看 `9_security` 了解安全加密通信
- 回顾 `1_hello_world` 理解基础通信流程