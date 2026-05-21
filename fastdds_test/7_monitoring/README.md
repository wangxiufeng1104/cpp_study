# 实时监控示例 - FastDDS 数据统计与监控

本示例演示如何使用 FastDDS 的监控功能，实时统计和显示数据传输情况。

## 概念说明

### 监控功能

FastDDS 提供了多种监控机制：

- **数据统计**: 接收样本数、吞吐量
- **连接状态**: 发布者/订阅者匹配状态
- **性能指标**: 延迟、丢包率
- **资源使用**: 内存、CPU 使用情况

### 实现方式

通过 `DataReaderListener` 监听器回调实现实时监控：

- `on_data_available`: 数据到达时触发
- `on_subscription_matched`: 发布者匹配状态变化

## 文件结构

```
7_monitoring/
├── CMakeLists.txt      # 构建配置
├── monitor.cpp         # 监控程序实现
└── README.md           # 本文件
```

## 编译

在项目根目录执行:

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## 运行

### 终端 1 - 启动监控程序

```bash
./7_monitoring_monitor
```

### 终端 2 - 启动发布者 (用于测试)

```bash
./1_hello_world_publisher
```

或者使用任何其他发布者示例。

## 预期输出

```
========================================
实时监控示例
========================================
监控初始化成功!

等待数据，按Ctrl+C退出...
================================================================================

[事件] 匹配到发布者 (匹配数: 1)
[2026-03-16 16:15:00] 总样本:     10 | 速率:   10.0 samples/s | 匹配: 1
[2026-03-16 16:15:01] 总样本:     20 | 速率:   10.0 samples/s | 匹配: 1
[2026-03-16 16:15:02] 总样本:     30 | 速率:   10.0 samples/s | 匹配: 1
...

[事件] 取消匹配发布者 (匹配数: 0)
监控退出
```

## 关键代码说明

### 监控类结构

```cpp
class Monitor
{
public:
    bool init();
    void run();

private:
    DomainParticipant* participant_;
    Subscriber* subscriber_;
    Topic* topic_;
    DataReader* reader_;
    TypeSupport type_;
    
    std::atomic<uint64_t> total_samples_;    // 总样本计数
    std::atomic<int16_t> matched_count_;     // 匹配的发布者数

    class MonitorListener : public DataReaderListener { ... } listener_;
};
```

### 数据接收统计

```cpp
void on_data_available(DataReader* reader) override
{
    SampleInfo info;
    HelloWorld hello;
    
    while (reader->take_next_sample(&hello, &info) == ReturnCode_t::RETCODE_OK)
    {
        if (info.valid_data)
        {
            parent_->total_samples_++;  // 原子计数
        }
    }
}
```

### 匹配状态监控

```cpp
void on_subscription_matched(DataReader*, const SubscriptionMatchedStatus& info) override
{
    parent_->matched_count_ = static_cast<int16_t>(info.current_count);
    
    if (info.current_count_change == 1)
        std::cout << "\n[事件] 匹配到发布者 (匹配数: " << info.current_count << ")" << std::endl;
    else if (info.current_count_change == -1)
        std::cout << "\n[事件] 取消匹配发布者 (匹配数: " << info.current_count << ")" << std::endl;
}
```

### 吞吐量计算

```cpp
void run()
{
    auto last_count = total_samples_;
    auto last_time = std::chrono::steady_clock::now();
    
    while (!g_shutdown)
    {
        sleep_ms(1000);
        
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count();
        
        if (elapsed > 0 && total_samples_ > last_count)
        {
            // 计算速率 (samples/s)
            double rate = (total_samples_ - last_count) * 1000.0 / elapsed;
            
            std::cout << "[" << get_timestamp() << "] "
                      << "总样本: " << std::setw(6) << total_samples_
                      << " | 速率: " << std::setw(6) << std::fixed << std::setprecision(1) << rate
                      << " samples/s | 匹配: " << matched_count_
                      << std::flush;
            
            last_count = total_samples_;
            last_time = now;
        }
    }
}
```

## 扩展监控功能

### 1. 延迟测量

通过在消息中携带时间戳来计算端到端延迟：

```cpp
// 发布者端
HelloWorld msg;
msg.timestamp(std::chrono::system_clock::now().time_since_epoch().count());
writer_->write(&msg);

// 订阅者端
void on_data_available(DataReader* reader) override
{
    HelloWorld msg;
    SampleInfo info;
    
    while (reader->take_next_sample(&msg, &info) == ReturnCode_t::RETCODE_OK)
    {
        if (info.valid_data)
        {
            auto now = std::chrono::system_clock::now().time_since_epoch().count();
            auto latency = now - msg.timestamp();
            std::cout << "延迟: " << latency << " ns" << std::endl;
        }
    }
}
```

### 2. DataReader 统计信息

```cpp
// 获取 DataReader 统计
DataReader* reader = ...;
eprosima::fastdds::statistics::DataCounters counters;
reader->get_counters(counters);

std::cout << "接收样本数: " << counters.sample_count << std::endl;
std::cout << "接收字节数: " << counters.byte_count << std::endl;
```

### 3. 参与者统计

```cpp
DomainParticipant* participant = ...;

// 获取匹配的发布者/订阅者数量
auto matched_publishers = participant->get_matched_publishers();
auto matched_subscribers = participant->get_matched_subscribers();
```

## 监控面板集成

可以将监控数据导出到外部系统：

### Prometheus 格式

```cpp
// 导出为 Prometheus 格式
std::string prometheus_metrics() 
{
    std::ostringstream oss;
    oss << "# HELP dds_samples_total Total samples received\n";
    oss << "# TYPE dds_samples_total counter\n";
    oss << "dds_samples_total " << total_samples_ << "\n";
    
    oss << "# HELP dds_throughput Current throughput\n";
    oss << "# TYPE dds_throughput gauge\n";
    oss << "dds_throughput " << current_rate_ << "\n";
    
    return oss.str();
}
```

### JSON 格式

```cpp
// 导出为 JSON 格式
nlohmann::json json_metrics()
{
    return {
        {"samples_total", total_samples_.load()},
        {"throughput", current_rate_},
        {"matched_publishers", matched_count_.load()},
        {"timestamp", get_timestamp()}
    };
}
```

## 应用场景

| 场景 | 监控指标 |
|------|----------|
| 性能测试 | 吞吐量、延迟 |
| 系统运维 | 连接状态、资源使用 |
| 故障诊断 | 丢包率、错误计数 |
| 容量规划 | 峰值流量、资源趋势 |

## 注意事项

1. **原子操作**: 使用 `std::atomic` 保证线程安全
2. **性能影响**: 监控逻辑应尽量轻量
3. **采样间隔**: 选择合适的统计间隔 (通常 1 秒)
4. **内存使用**: 长时间运行需注意历史数据存储

## 下一步

- 查看 `8_discovery` 了解参与者发现机制
- 查看 `9_security` 了解安全加密通信