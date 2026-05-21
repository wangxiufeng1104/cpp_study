# FastDDS 测试示例

FastDDS (Fast DDS) 是 eProsima 公司实现的开源 DDS (Data Distribution Service) 中间件，符合 OMG DDS 标准。本项目包含多个示例程序，演示 FastDDS 的核心功能。

## FastDDS 技术原理

### DDS 架构

DDS (Data Distribution Service) 是 OMG 制定的数据分发服务标准，采用发布-订阅模式实现进程间通信。

```
┌─────────────────────────────────────────────────────────────────────┐
│                          Application Layer                          │
├─────────────────────────────────────────────────────────────────────┤
│                         DDS API (DCPS)                              │
│  ┌──────────────────┐  ┌──────────────┐  ┌──────────────┐           │
│  │ DomainParticipant│  │   Publisher  │  │  Subscriber  │           │
│  └──────────────────┘  └──────────────┘  └──────────────┘           │
│  ┌──────────────────┐  ┌──────────────┐  ┌──────────────┐           │
│  │      Topic       │  │  DataWriter  │  │  DataReader  │           │
│  └──────────────────┘  └──────────────┘  └──────────────┘           │
├─────────────────────────────────────────────────────────────────────┤
│                      DDS Infrastructure                             │
│  ┌──────────────────┐  ┌──────────────┐  ┌──────────────┐           │
│  │    Discovery     │  │    Routing   │  │     QoS      │           │
│  └──────────────────┘  └──────────────┘  └──────────────┘           │
├─────────────────────────────────────────────────────────────────────┤
│                      RTPS Protocol Layer                            │
│  ┌────────────────────────────────────────────────────────────────┐ │
│  │                    RTPS Wire Protocol                          │ │
│  └────────────────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────────────┤
│                         Transport Layer                             │
│  ┌────────┐  ┌────────┐  ┌──────────┐  ┌────────────────────────┐   │
│  │  UDP   │  │  TCP   │  │ SharedMem│  │   Custom Transport     │   │
│  └────────┘  └────────┘  └──────────┘  └────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
```

### 核心概念

#### 1. DomainParticipant (域参与者)

域参与者是 DDS 应用的入口点，代表同一通信域中的节点。

- **Domain ID**: 0-232 的整数，同一 Domain ID 的参与者才能通信
- **隔离性**: 不同 Domain 之间完全隔离
- **资源管理**: 管理该域内的所有实体

#### 2. Topic (主题)

主题定义数据类型和通信通道。

- **类型安全**: 数据类型必须在编译时定义
- **唯一标识**: Topic 名称 + 数据类型 = 唯一主题
- **QoS 策略**: 每个主题可以配置不同的 QoS

#### 3. Publisher/Subscriber (发布者/订阅者)

```
Publisher (发布者)
    │
    ├── DataWriter (数据写入器)
    │       │
    │       └── write(data) → 发送到 Topic
    │
    └── 管理 DataWriter 的生命周期

Subscriber (订阅者)
    │
    ├── DataReader (数据读取器)
    │       │
    │       ├── take(data) → 非阻塞读取
    │       └── read(data) → 阻塞读取
    │
    └── 管理 DataReader 的生命周期
```

#### 4. DataWriter/DataReader

实际执行数据读写的实体。

- **类型化接口**: 每个实例绑定特定数据类型
- **监听器模式**: 通过回调接收数据到达通知
- **QoS 配置**: 可独立配置 QoS 策略

### RTPS 协议

FastDDS 使用 RTPS (Real-Time Publish-Subscribe) 协议作为底层通信协议。

#### RTPS 实体映射

| DDS 实体 | RTPS 实体 |
|----------|-----------|
| DomainParticipant | RTPSParticipant |
| DataWriter | RTPSWriter |
| DataReader | RTPSReader |
| Topic | - (通过类型和名称匹配) |

#### RTPS 消息类型

| 消息类型 | 说明 |
|----------|------|
| DATA | 数据消息，携带用户数据 |
| HEARTBEAT | 心跳消息，通知可用数据 |
| ACKNACK | 确认/否定确认 |
| GAP | 通知缺失数据 |
| INFO_TS | 时间戳信息 |
| INFO_SRC | 源信息 |

### 发现机制

#### SPDP (Simple Participant Discovery Protocol)

参与者发现协议，自动发现同一域中的其他参与者。

```
参与者 A                    参与者 B
    │                           │
    │ ──────── SPDP ─────────► │
    │ ◄─────── SPDP ────────── │
    │                           │
    │ (建立元数据通道)           │
```

#### SEDP (Simple Endpoint Discovery Protocol)

端点发现协议，在参与者发现后交换 DataReader/DataWriter 信息。

```
参与者 A                    参与者 B
    │                           │
    │ ──────── SEDP ─────────► │
    │   (Publication Writer)   │
    │                           │
    │ ◄─────── SEDP ────────── │
    │   (Subscription Writer)  │
    │                           │
    │ (匹配 DataReader/Writer)  │
```

### QoS 策略详解

#### Reliability (可靠性)

| 模式 | 延迟 | 可靠性 | 适用场景 |
|------|------|--------|----------|
| RELIABLE | 较高 | 100% | 金融交易、控制命令 |
| BEST_EFFORT | 低 | 尽力 | 视频流、传感器数据 |

#### Durability (持久性)

| 模式 | 说明 |
|------|------|
| VOLATILE | 数据不持久化，只发送给当前订阅者 |
| TRANSIENT_LOCAL | 数据在发布者端缓存，新订阅者可获取 |
| TRANSIENT | 数据在 DDS 中间件中持久化 |
| PERSISTENT | 数据持久化到存储介质 |

#### History (历史深度)

| 模式 | 说明 |
|------|------|
| KEEP_LAST(N) | 保留最近 N 条数据 |
| KEEP_ALL | 保留全部数据 (受资源限制) |

### 传输层

| 传输协议 | 特点 | 适用场景 |
|----------|------|----------|
| UDP | 无连接、支持多播、低延迟 | 局域网、实时数据 |
| TCP | 面向连接、穿透防火墙 | 广域网、云环境 |
| SharedMem | 零拷贝、极低延迟 | 同主机进程通信 |

### 数据序列化

FastDDS 使用 Fast-CDR 进行高效序列化：

- **CDR 编码**: OMG Common Data Representation 标准
- **字节序**: 支持大端和小端
- **零拷贝**: 尽可能避免数据复制

## 项目结构

```
fastdds_test/
├── CMakeLists.txt              # 主CMake配置
├── common/                     # 公共代码
│   ├── include/Common.hpp      # 公共头文件
│   └── src/Common.cpp          # 公共实现
├── 1_hello_world/              # 基础发布订阅示例
├── 2_custom_types/             # 自定义数据类型示例
├── 3_request_reply/            # 请求-应答模式示例
├── 4_qos_config/               # QoS配置示例
├── 5_data_sharing/             # DataSharing共享内存示例
├── 6_tcp_transport/            # TCP传输示例
├── 7_monitoring/               # 实时监控示例
├── 8_discovery/                # 发现机制示例
└── 9_security/                 # 安全机制示例
```

## 依赖

- CMake 3.16+
- C++17 编译器
- FastDDS 3.1.0+ (自动下载)

## 编译

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## 示例说明

### 1. Hello World (基础发布订阅)

最简单的 DDS 示例，演示基本的发布/订阅模式。

```bash
# 终端1 - 运行订阅者
./1_hello_world_subscriber

# 终端2 - 运行发布者
./1_hello_world_publisher
```

### 2. 自定义数据类型

演示如何定义和使用复杂的 IDL 数据类型。

```bash
./2_custom_types_subscriber
./2_custom_types_publisher
```

### 3. 请求-应答模式

实现 RPC 风格的通信模式。

```bash
./request_reply_replier   # 服务端
./request_reply_requester # 客户端
```

### 4. QoS 配置

演示各种 QoS 策略配置。

```bash
./qos_subscriber
./qos_publisher
```

### 5. DataSharing 共享内存

使用共享内存进行高效数据传输。

```bash
./data_sharing_subscriber
./data_sharing_publisher
```

### 6. TCP 传输

配置 TCP 作为传输协议。

```bash
./tcp_publisher 5100        # 监听端口 5100
./tcp_subscriber 127.0.0.1 5100  # 连接
```

### 7. 实时监控

监控数据吞吐量和连接状态。

```bash
./monitoring
```

### 8. 发现机制

监控网络中的参与者发现。

```bash
./discovery_participant
```

### 9. 安全机制

启用 DDS Security 加密通信。

```bash
./secure_publisher ./certs
./secure_subscriber ./certs
```

## IDL 数据类型

项目使用以下 IDL 文件定义数据类型：

### HelloWorld.idl
```idl
module HelloWorld
{
    struct Msg
    {
        unsigned long index;
        string message;
    };
};
```

### CustomData.idl
```idl
module CustomData
{
    struct SensorData
    {
        unsigned long id;
        string name;
        float value;
        unsigned long long timestamp;
    };
};
```

## 常见问题

### Q: 编译时找不到 FastDDS?

A: 项目使用 FetchContent 自动下载 FastDDS，确保网络连接正常。

### Q: 发布者和订阅者无法匹配?

A: 检查是否在同一 Domain ID (默认为 0)，并确保主题名称一致。

### Q: 安全示例运行失败?

A: 需要正确配置证书文件。参考 FastDDS Security 文档生成证书。

## 参考资源

- [FastDDS 官方文档](https://fast-dds.docs.eprosima.com/)
- [OMG DDS 规范](https://www.omg.org/spec/DDS/)
- [RTPS 协议](https://www.omg.org/spec/DDSI-RTPS/)