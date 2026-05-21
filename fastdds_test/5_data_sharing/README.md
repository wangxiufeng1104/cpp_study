# DataSharing 示例 - FastDDS 共享内存通信

本示例演示 FastDDS 的 DataSharing 特性，使用共享内存实现高效的进程间数据传输。

## 概念说明

### 什么是 DataSharing?

DataSharing 是 FastDDS 的一项优化技术，允许同一主机上的发布者和订阅者通过共享内存直接交换数据，避免了数据的序列化/反序列化和网络传输开销。

### 工作原理

```
传统方式:
发布者 → 序列化 → 网络传输 → 反序列化 → 订阅者

DataSharing:
发布者 → 写入共享内存 → 订阅者直接读取
```

### 优势

| 特性 | 传统传输 | DataSharing |
|------|----------|-------------|
| 延迟 | 较高 | 极低 |
| CPU 使用 | 较高 (序列化) | 低 (零拷贝) |
| 内存使用 | 多份拷贝 | 共享内存 |
| 跨主机 | 支持 | 不支持 |

## 文件结构

```
5_data_sharing/
├── CMakeLists.txt      # 构建配置
├── publisher.cpp       # 发布者实现 (启用DataSharing)
├── subscriber.cpp      # 订阅者实现 (启用DataSharing)
└── README.md           # 本文件
```

## DataSharing 配置

### 启用 DataSharing

```cpp
// 自动模式 - FastDDS 自动选择是否使用共享内存
topic_qos.data_sharing().automatic();

// 指定共享内存目录
topic_qos.data_sharing().directories.push_back("/dev/shm/fastdds_datasharing");

// 配置限制
topic_qos.data_sharing().max_domains = 1;
topic_qos.data_sharing().max_participants_per_domain = 10;
```

### DataSharing 模式

| 模式 | 说明 |
|------|------|
| `automatic()` | 自动选择，优先使用共享内存 |
| `off()` | 禁用共享内存 |
| `on(directories)` | 强制启用，指定共享内存目录 |

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
./5_data_sharing_subscriber
```

### 终端 2 - 启动发布者

```bash
./5_data_sharing_publisher
```

## 预期输出

### 发布者输出

```
========================================
DataSharing共享内存 发布者示例
========================================
DataSharing发布者初始化成功!
DataSharing配置: AUTOMATIC
说明: 使用共享内存进行高效数据传输

等待订阅者连接...
匹配到DataSharing订阅者 (匹配数: 1)
[2026-03-16 16:05:00] 已发布(共享内存): DataSharing Message #0
...
```

### 订阅者输出

```
========================================
DataSharing共享内存 订阅者示例
========================================
DataSharing订阅者初始化成功!
DataSharing配置: AUTOMATIC

匹配到DataSharing发布者 (匹配数: 1)
[2026-03-16 16:05:00] 收到(共享内存): DataSharing Message #0
...
```

## 关键代码说明

### 发布者 DataSharing 配置

```cpp
// 配置 Topic QoS - 启用 DataSharing
TopicQos topic_qos;
topic_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
topic_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
topic_qos.history().kind = KEEP_LAST_HISTORY_QOS;
topic_qos.history().depth = 10;

// DataSharing 配置
topic_qos.data_sharing().automatic();
topic_qos.data_sharing().max_domains = 1;
topic_qos.data_sharing().max_participants_per_domain = 10;

topic_ = participant_->create_topic("DataSharingTopic", type_.get_type_name(), topic_qos);
```

### DataWriter DataSharing 配置

```cpp
DataWriterQos writer_qos;
writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
writer_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
writer_qos.history().kind = KEEP_LAST_HISTORY_QOS;
writer_qos.history().depth = 10;

// 启用 DataSharing
writer_qos.data_sharing().automatic();

writer_ = publisher_->create_datawriter(topic_, writer_qos, &listener_);
```

## 使用场景

### 适合 DataSharing 的场景

- 同一主机上的进程间通信
- 高吞吐量数据传输 (如视频、点云)
- 低延迟要求的实时系统
- CPU 资源受限的嵌入式系统

### 不适合 DataSharing 的场景

- 跨主机通信
- 数据需要持久化
- 对数据顺序有严格要求的场景 (配合 KEEP_ALL)

## 性能对比

典型场景下的性能对比 (仅供参考):

| 场景 | 传统传输 | DataSharing |
|------|----------|-------------|
| 1KB 消息延迟 | ~100μs | ~10μs |
| 1MB 消息延迟 | ~10ms | ~100μs |
| CPU 使用 (高吞吐) | 100% | ~30% |
| 内存使用 | 2x 数据大小 | 1x 数据大小 |

## 注意事项

1. **同主机限制**: DataSharing 仅适用于同一主机上的进程
2. **内存要求**: 共享内存需要足够的系统共享内存空间
3. **兼容性**: 发布者和订阅者都需要启用 DataSharing
4. **回退机制**: 如果 DataSharing 不可用，FastDDS 会自动回退到传统传输

## 系统配置

在某些系统上，可能需要调整共享内存限制:

```bash
# 查看当前共享内存大小
cat /proc/sys/kernel/shmmax

# 临时调整 (需要 root 权限)
sudo sysctl -w kernel.shmmax=1073741824  # 1GB

# 永久调整，编辑 /etc/sysctl.conf
kernel.shmmax = 1073741824
```

## 下一步

- 查看 `6_tcp_transport` 了解 TCP 传输配置
- 查看 `7_monitoring` 了解实时监控功能