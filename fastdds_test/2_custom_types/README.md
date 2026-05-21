# 自定义数据类型示例 - FastDDS 复杂数据结构

本示例演示如何在 FastDDS 中定义和使用复杂的 IDL 数据类型。

## 概念说明

### IDL (Interface Definition Language)

IDL 是 OMG (Object Management Group) 定义的标准语言，用于描述数据类型和接口。FastDDS 使用 IDL 定义的数据类型自动生成 C++ 代码。

### 支持的数据类型

| 类型 | 说明 | 示例 |
|------|------|------|
| 基本类型 | 整数、浮点、布尔等 | `long`, `double`, `boolean` |
| 字符串 | 可变长度字符串 | `string` |
| 枚举 | 命名常量集合 | `enum Status { ... }` |
| 结构体 | 复合数据类型 | `struct Position { ... }` |
| 序列 | 动态数组 | `sequence<long, 100>` |
| 数组 | 固定长度数组 | `long data[10]` |
| 联合体 | 多选一类型 | `union Value switch(...) { ... }` |
| Map | 键值对集合 | `map<string, long>` |

## 文件结构

```
2_custom_types/
├── CMakeLists.txt          # 构建配置
├── idl/
│   └── CustomData.idl      # 自定义数据类型定义
├── publisher.cpp           # 发布者实现
├── subscriber.cpp          # 订阅者实现
└── README.md               # 本文件
```

## IDL 数据类型定义

```idl
module CustomData
{
    // 枚举类型
    enum Status
    {
        STATUS_UNKNOWN,
        STATUS_INITIALIZING,
        STATUS_RUNNING,
        STATUS_PAUSED,
        STATUS_STOPPED,
        STATUS_ERROR
    };

    // 嵌套结构体 - 位置信息
    struct Position
    {
        double x;
        double y;
        double z;
    };

    // 嵌套结构体 - 速度信息
    struct Velocity
    {
        double vx;
        double vy;
        double vz;
    };

    // 嵌套结构体 - 传感器数据
    struct SensorData
    {
        double temperature;
        double humidity;
        double pressure;
    };

    // 主消息结构
    struct DeviceMessage
    {
        unsigned long id;                    // 设备ID
        string device_name;                  // 设备名称
        Status status;                       // 设备状态
        Position position;                   // 位置信息
        Velocity velocity;                   // 速度信息
        SensorData sensor;                   // 传感器数据
        sequence<long, 100> data_array;      // 数据数组(最大100元素)
        unsigned long long timestamp;        // 时间戳
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

IDL 文件会自动通过 `fastddsgen` 工具生成 C++ 代码，生成的文件位于 `build/2_custom_types/` 目录。

## 运行

### 终端 1 - 启动订阅者

```bash
./2_custom_types_subscriber
```

### 终端 2 - 启动发布者

```bash
./2_custom_types_publisher
```

## 预期输出

### 发布者输出

```
========================================
FastDDS 自定义类型 发布者示例
========================================
自定义类型发布者初始化成功!
等待订阅者连接...
[2026-03-16 15:55:00] 已发布: Device #0, Status=RUNNING, Pos=(1.0, 2.0, 3.0)
...
```

### 订阅者输出

```
========================================
FastDDS 自定义类型 订阅者示例
========================================
自定义类型订阅者初始化成功!
[2026-03-16 15:55:00] 收到消息:
  设备ID: 0
  名称: Device-0
  状态: RUNNING (2)
  位置: (1.0, 2.0, 3.0)
  速度: (0.1, 0.2, 0.3)
  传感器: temp=25.5°C, humidity=60.2%, pressure=1013.25hPa
...
```

## 关键代码说明

### 设置枚举值

```cpp
DeviceMessage msg;
msg.status(CustomData::Status::STATUS_RUNNING);
```

### 设置嵌套结构体

```cpp
CustomData::Position pos;
pos.x(1.0);
pos.y(2.0);
pos.z(3.0);
msg.position(pos);
```

### 设置序列类型

```cpp
msg.data_array().resize(10);
for (int i = 0; i < 10; i++)
{
    msg.data_array()[i] = i * 10;
}
```

### 读取嵌套数据

```cpp
const auto& pos = msg.position();
std::cout << "Position: (" << pos.x() << ", " << pos.y() << ", " << pos.z() << ")" << std::endl;
```

## IDL 类型映射

| IDL 类型 | C++ 类型 |
|----------|----------|
| `boolean` | `bool` |
| `char` | `char` |
| `octet` | `uint8_t` |
| `short` | `int16_t` |
| `unsigned short` | `uint16_t` |
| `long` | `int32_t` |
| `unsigned long` | `uint32_t` |
| `long long` | `int64_t` |
| `unsigned long long` | `uint64_t` |
| `float` | `float` |
| `double` | `double` |
| `string` | `std::string` |
| `sequence<T, N>` | `std::vector<T>` |

## 注意事项

1. **内存管理**: 序列类型使用 `resize()` 预分配内存
2. **命名空间**: 生成的 C++ 代码位于与 IDL module 同名的命名空间中
3. **访问器方法**: 生成的类使用 getter/setter 方法访问字段

## 下一步

- 查看 `3_request_reply` 了解请求-应答模式
- 查看 `4_qos_config` 了解 QoS 配置