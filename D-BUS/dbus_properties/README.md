# D-Bus 属性 (Properties) 接口

演示 D-Bus 标准接口 `org.freedesktop.DBus.Properties` 的使用：
Get / Set / GetAll 方法，以及 VARIANT 类型的序列化与反序列化。

## 核心概念

- **Properties 接口**: D-Bus 规范定义的标准接口，提供属性的 CRUD 操作
- **Get(iface, prop) → VARIANT**: 读取单个属性值
- **Set(iface, prop, VARIANT)**: 写入单个属性值
- **GetAll(iface) → DICT<STRING,VARIANT>**: 读取接口所有属性
- **VARIANT 类型**: 需要 `dbus_message_iter_open_container()` / `dbus_message_iter_recurse()` 进行序列化/反序列化
- **DICT_ENTRY**: 键值对，用于 GetAll 返回值

## 运行

```bash
# 终端 1: 启动服务端
./bin/dbus_properties_server

# 终端 2: 客户端执行 Get / Set / GetAll
./bin/dbus_properties_client
```

## 输出示例

```
[server] Properties service listening on com.example.DBusTutorial at /com/example/Tutorial
[server] Properties: PowerStatus=true, Volume=50
[client] GetAll():
  PowerStatus = true
  Volume = 50
[client] Get(PowerStatus) = true (bool)
[client] Get(Volume) = 50 (int32)
[client] Set(PowerStatus, false) OK
[server] PowerStatus set to false
[client] Get(PowerStatus) = false (bool)
[client] GetAll():
  PowerStatus = false
  Volume = 50
```
