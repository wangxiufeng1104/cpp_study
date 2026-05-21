# QoS 配置示例 - FastDDS 服务质量策略

本示例演示如何配置和使用 FastDDS 的 QoS (Quality of Service) 策略。

## 概念说明

### 什么是 QoS?

QoS (Quality of Service) 是 DDS 的核心特性之一，允许用户精确控制数据传输的行为。通过 QoS 策略，可以配置：
- 数据传输的可靠性
- 数据的持久性
- 资源使用限制
- 时间约束

### 常用 QoS 策略

| 策略 | 说明 | 可选值 |
|------|------|--------|
| **Reliability** | 数据传输可靠性 | BEST_EFFORT (尽力传输，不保证到达), RELIABLE (可靠传输，保证数据到达) |
| **Durability** | 数据持久性 | VOLATILE (不持久化), TRANSIENT_LOCAL (本地缓存历史数据), TRANSIENT (中间件持久化), PERSISTENT (持久化到存储介质) |
| **History** | 历史数据保留 | KEEP_LAST(N) (保留最近N条), KEEP_ALL (保留所有数据，受资源限制) |
| **Deadline** | 发送/接收截止时间 | 时间间隔 (如 1000ms，超时触发回调) |
| **Lifespan** | 数据有效期 | 时间长度 (如 5000ms，过期数据将被丢弃) |
| **ResourceLimits** | 资源限制 | 最大样本数、最大实例数、每个实例最大样本数等 |

## 文件结构

```
4_qos_config/
├── CMakeLists.txt      # 构建配置
├── publisher.cpp       # 发布者实现 (带QoS配置)
├── subscriber.cpp      # 订阅者实现 (带QoS配置)
└── README.md           # 本文件
```

## QoS 配置详解

### 1. Reliability (可靠性)

```cpp
// 可靠传输 - 保证数据到达
writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

// 尽力传输 - 不保证到达，但延迟更低
writer_qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
```

| 模式 | 特点 | 适用场景 |
|------|------|----------|
| RELIABLE | 保证数据到达，可能重传 | 重要数据、控制命令 |
| BEST_EFFORT | 不保证到达，低延迟 | 传感器数据、视频流 |

### 2. Durability (持久性)

```cpp
// 本地持久化 - 新订阅者可获取历史数据
writer_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;

// 不持久化 - 只发送给已连接的订阅者
writer_qos.durability().kind = VOLATILE_DURABILITY_QOS;
```

| 模式 | 说明 |
|------|------|
| VOLATILE | 数据不持久化，只发送给当前订阅者 |
| TRANSIENT_LOCAL | 数据在发布者端缓存，新订阅者可获取 |
| TRANSIENT | 数据在 DDS 中间件中持久化 |
| PERSISTENT | 数据持久化到存储介质 |

### 3. History (历史深度)

```cpp
// 保留最近N条数据
writer_qos.history().kind = KEEP_LAST_HISTORY_QOS;
writer_qos.history().depth = 10;

// 保留所有数据 (受资源限制约束)
writer_qos.history().kind = KEEP_ALL_HISTORY_QOS;
```

### 4. Deadline (截止时间)

```cpp
// 发布者必须每秒发送一条消息
writer_qos.deadline().period = Duration_t(1, 0);  // 1秒

// 监听截止时间错过事件
void on_offered_deadline_missed(DataWriter*, const OfferedDeadlineMissedStatus& status) override
{
    std::cout << "错过截止时间! 总次数: " << status.total_count << std::endl;
}
```

### 5. Lifespan (生命周期)

```cpp
// 数据在5秒后过期
writer_qos.lifespan().duration = Duration_t(5, 0);  // 5秒
```

## 编译

在项目根目录执行:

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## 运行

### 终端 1 - 启动订阅者

```bash
./4_qos_config_subscriber
```

### 终端 2 - 启动发布者

```bash
./4_qos_config_publisher
```

## 预期输出

### 发布者输出

```
========================================
QoS配置 发布者示例
========================================
QoS发布者初始化成功!
QoS配置:
  Reliability: RELIABLE
  Durability: TRANSIENT_LOCAL
  History: KEEP_LAST (depth=10)
  Deadline: 1000ms
  Lifespan: 5000ms

等待订阅者连接...
匹配到订阅者 (匹配数: 1)
[2026-03-16 16:00:00] 已发布: QoS Message #0
...
```

### 订阅者输出

```
========================================
QoS配置 订阅者示例
========================================
QoS订阅者初始化成功!
QoS配置:
  Reliability: RELIABLE
  Durability: TRANSIENT_LOCAL
  History: KEEP_LAST (depth=10)
  Deadline: 1000ms

匹配到发布者 (匹配数: 1)
[2026-03-16 16:00:00] 收到: QoS Message #0
...
```

## QoS 兼容性

发布者和订阅者的 QoS 必须兼容才能匹配：

### Reliability 兼容性

| 发布者 | 订阅者 | 兼容性 |
|--------|--------|--------|
| BEST_EFFORT | BEST_EFFORT | ✅ |
| BEST_EFFORT | RELIABLE | ❌ |
| RELIABLE | BEST_EFFORT | ✅ |
| RELIABLE | RELIABLE | ✅ |

### Durability 兼容性

订阅者的 Durability 要求不能高于发布者提供的。

## 关键代码示例

### 完整的 DataWriter QoS 配置

```cpp
DataWriterQos writer_qos;

// 可靠性: 可靠传输
writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

// 持久性: 订阅者连接后可以获取之前发送的数据
writer_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;

// 历史深度: 保留最近10条消息
writer_qos.history().kind = KEEP_LAST_HISTORY_QOS;
writer_qos.history().depth = 10;

// 截止时间: 每1000ms必须发送一条消息
writer_qos.deadline().period = Duration_t(1, 0);

// 生命周期: 数据在5000ms后过期
writer_qos.lifespan().duration = Duration_t(5, 0);
```

### 完整的 DataReader QoS 配置

```cpp
DataReaderQos reader_qos;

// 可靠性: 请求可靠传输
reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;

// 持久性: 请求历史数据
reader_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;

// 历史深度
reader_qos.history().kind = KEEP_LAST_HISTORY_QOS;
reader_qos.history().depth = 10;

// 期望的截止时间
reader_qos.deadline().period = Duration_t(1, 0);
```

## 应用场景

| 场景 | 推荐配置 |
|------|----------|
| 传感器数据 | BEST_EFFORT + VOLATILE + KEEP_LAST(1) |
| 控制命令 | RELIABLE + TRANSIENT_LOCAL |
| 状态同步 | RELIABLE + TRANSIENT_LOCAL + KEEP_LAST(1) |
| 数据记录 | RELIABLE + KEEP_ALL |
| 视频流 | BEST_EFFORT + KEEP_LAST(depth) |

## 注意事项

1. **QoS 兼容性**: 发布者和订阅者的 QoS 必须兼容才能通信
2. **资源消耗**: RELIABLE 和 TRANSIENT_LOCAL 会消耗更多内存
3. **Deadline 监控**: 设置 Deadline 后应处理错过截止时间的回调
4. **性能影响**: 高可靠性配置会增加延迟

## 下一步

- 查看 `5_data_sharing` 了解共享内存高效传输
- 查看 `6_tcp_transport` 了解 TCP 传输配置