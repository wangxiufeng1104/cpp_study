# HelloWorld 示例 - FastDDS 基础发布订阅

本示例演示 FastDDS 最基础的发布/订阅模式实现。

## 概念说明

### DDS 核心组件

1. **DomainParticipant**: DDS 应用的入口点，代表同一个通信域中的参与者
2. **Topic**: 数据通道，定义数据类型和主题名称
3. **Publisher**: 发布者，负责发布数据
4. **Subscriber**: 订阅者，负责接收数据
5. **DataWriter**: 数据写入器，与 Publisher 关联，实际执行数据发布
6. **DataReader**: 数据读取器，与 Subscriber 关联，实际接收数据

### 通信流程

```
发布者端:
DomainParticipant → Publisher → DataWriter → Topic → 网络

订阅者端:
网络 → Topic → Subscriber → DataReader → 应用
```

## 文件结构

```
1_hello_world/
├── CMakeLists.txt      # 构建配置
├── HelloWorld.idl      # 数据类型定义
├── publisher.cpp       # 发布者实现
├── subscriber.cpp      # 订阅者实现
└── README.md           # 本文件
```

## IDL 数据类型

```idl
module HelloWorld
{
    struct Msg
    {
        unsigned long index;    // 消息序号
        string message;         // 消息内容
    };
};
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
./1_hello_world_subscriber
```

### 终端 2 - 启动发布者

```bash
./1_hello_world_publisher
```

## 预期输出

### 发布者输出

```
========================================
FastDDS HelloWorld 发布者示例
========================================
HelloWorld发布者初始化成功!
等待订阅者连接...
发布者匹配到订阅者: 当前匹配数 = 1
[2026-03-16 15:55:00] 已发布: index=0, message="Hello World! #0"
[2026-03-16 15:55:01] 已发布: index=1, message="Hello World! #1"
...
```

### 订阅者输出

```
========================================
FastDDS HelloWorld 订阅者示例
========================================
HelloWorld订阅者初始化成功!
订阅者匹配到发布者: 当前匹配数 = 1
[2026-03-16 15:55:00] 收到消息: index=0, message="Hello World! #0" (总计: 1 条)
[2026-03-16 15:55:01] 收到消息: index=1, message="Hello World! #1" (总计: 2 条)
...
```

## 关键代码说明

### 发布者初始化流程

```cpp
// 1. 创建 DomainParticipant
participant_ = DomainParticipantFactory::get_instance()->create_participant(0, participant_qos);

// 2. 注册数据类型
type_.register_type(participant_);

// 3. 创建 Topic
topic_ = participant_->create_topic("HelloWorldTopic", type_.get_type_name(), TOPIC_QOS_DEFAULT);

// 4. 创建 Publisher
publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

// 5. 创建 DataWriter (带监听器)
writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, listener_);
```

### 订阅者数据接收

通过实现 `DataReaderListener` 接口接收数据:

```cpp
void on_data_available(DataReader* reader) override
{
    SampleInfo info;
    HelloWorld hello;
    while (reader->take_next_sample(&hello, &info) == ReturnCode_t::RETCODE_OK)
    {
        if (info.valid_data)
        {
            // 处理接收到的数据
        }
    }
}
```

## 注意事项

1. **Domain ID**: 默认使用 Domain ID 0，同一 Domain 内的参与者才能通信
2. **Topic 名称**: 发布者和订阅者必须使用相同的 Topic 名称
3. **启动顺序**: 先启动订阅者可以确保不丢失早期消息（配合 TRANSIENT_LOCAL QoS）

## 下一步

- 查看 `2_custom_types` 了解如何定义复杂数据类型
- 查看 `4_qos_config` 了解 QoS 配置对通信的影响