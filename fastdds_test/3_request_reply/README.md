# 请求-应答模式示例 - FastDDS RPC 风格通信

本示例演示如何使用 FastDDS 实现请求-应答 (Request-Reply) 模式，类似于 RPC 的通信方式。

## 概念说明

### 请求-应答模式

请求-应答模式是一种双向通信模式，其中：
- **Requester (请求者)**: 发送请求并等待响应
- **Replier (应答者)**: 接收请求并发送响应

### 实现方式

在 DDS 中，请求-应答模式通过两个独立的 Topic 实现：
1. **Request Topic**: 请求者发布，应答者订阅
2. **Reply Topic**: 应答者发布，请求者订阅

```
┌───────────┐    Request Topic    ┌───────────┐
│ Requester │ ──────────────────► │  Replier  │
│           │                     │           │
│           │ ◄────────────────── │           │
└───────────┘    Reply Topic      └───────────┘
```

## 文件结构

```
3_request_reply/
├── CMakeLists.txt          # 构建配置
├── idl/
│   └── RequestReply.idl    # 请求/应答数据类型定义
├── requester.cpp           # 请求者实现
├── replier.cpp             # 应答者实现
└── README.md               # 本文件
```

## IDL 数据类型定义

```idl
module RequestReply
{
    // 请求消息
    struct AddRequest
    {
        long a;     // 第一个操作数
        long b;     // 第二个操作数
    };

    // 应答消息
    struct AddReply
    {
        long result;        // 计算结果
        boolean success;    // 是否成功
        string message;     // 附加消息
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

### 终端 1 - 启动应答者 (服务端)

```bash
./3_request_reply_replier
```

### 终端 2 - 启动请求者 (客户端)

```bash
./3_request_reply_requester
```

## 预期输出

### 应答者输出

```
========================================
请求-应答模式 应答者示例
========================================
应答者初始化成功!
等待请求...

[2026-03-16 15:55:00] 收到请求: a=10, b=20
[2026-03-16 15:55:00] 发送响应: result=30, success=true
...
```

### 请求者输出

```
========================================
请求-应答模式 请求者示例
========================================
请求者初始化成功!

[2026-03-16 15:55:00] 发送请求: a=10, b=20
[2026-03-16 15:55:00] 收到响应: result=30, message="Success"
...
```

## 关键代码说明

### 应答者 (Replier)

应答者同时作为订阅者(接收请求)和发布者(发送响应):

```cpp
// 监听请求
void on_data_available(DataReader* reader) override
{
    AddRequest request;
    SampleInfo info;
    
    while (reader->take_next_sample(&request, &info) == ReturnCode_t::RETCODE_OK)
    {
        if (info.valid_data)
        {
            // 处理请求
            AddReply reply;
            reply.result(request.a() + request.b());
            reply.success(true);
            reply.message("Success");
            
            // 发送响应
            writer_->write(&reply);
        }
    }
}
```

### 请求者 (Requester)

请求者同时作为发布者(发送请求)和订阅者(接收响应):

```cpp
// 发送请求
bool send_request(int32_t a, int32_t b)
{
    AddRequest request;
    request.a(a);
    request.b(b);
    return writer_->write(&request) == ReturnCode_t::RETCODE_OK;
}

// 接收响应 (通过 DataReaderListener)
void on_data_available(DataReader* reader) override
{
    AddReply reply;
    SampleInfo info;
    
    while (reader->take_next_sample(&reply, &info) == ReturnCode_t::RETCODE_OK)
    {
        if (info.valid_data)
        {
            std::cout << "收到响应: result=" << reply.result() << std::endl;
        }
    }
}
```

## 应用场景

| 场景 | 说明 |
|------|------|
| 远程过程调用 | 调用远程服务并获取结果 |
| 查询服务 | 查询数据库或缓存 |
| 配置管理 | 获取或设置远程配置 |
| 设备控制 | 发送命令并确认执行结果 |

## 与标准 DDS 的区别

| 特性 | 标准 Pub/Sub | Request-Reply |
|------|-------------|---------------|
| 通信方向 | 单向 | 双向 |
| 耦合度 | 松耦合 | 较紧密 |
| 响应时间 | 不保证 | 可等待响应 |
| 典型用途 | 事件通知 | 远程调用 |

## 注意事项

1. **超时处理**: 请求者应设置合理的等待超时
2. **请求标识**: 实际应用中应添加请求 ID 以匹配请求和响应
3. **错误处理**: 处理应答者不可用或响应超时的情况
4. **并发**: 应答者可能需要处理多个并发请求

## 扩展建议

在实际应用中，可以考虑添加以下功能：

```cpp
// 带请求ID的请求结构
struct Request
{
    unsigned long request_id;  // 唯一请求标识
    // ... 其他字段
};

// 带请求ID的响应结构
struct Reply
{
    unsigned long request_id;  // 对应的请求ID
    // ... 其他字段
};
```

## 下一步

- 查看 `4_qos_config` 了解 QoS 配置
- 查看 `5_data_sharing` 了解共享内存通信