# D-Bus 基础 — 方法调用

演示 D-Bus 最基本的 RPC 模型：服务端导出方法，客户端远程调用。

## 核心概念

- **Connection**: `dbus_bus_get(DBUS_BUS_SESSION, &err)` 连接到 Session Bus
- **Service Name**: 类似域名 `com.example.DBusTutorial`，标识服务进程
- **Object Path**: `/com/example/Tutorial`，标识导出对象
- **Interface**: `dbus_message_is_method_call()` 匹配方法和接口
- **方法调用**: 客户端 `dbus_message_new_method_call()` → 构造请求
  → `dbus_connection_send_with_reply_and_block()` 同步调用
- **服务端分发**: `dbus_connection_read_write_dispatch()` 在事件循环中读取和分发消息

## 运行

```bash
# 终端 1: 启动服务端
./bin/dbus_server

# 终端 2: 调用客户端
./bin/dbus_client
```

## 输出示例

```
[server] Listening on com.example.DBusTutorial at /com/example/Tutorial
[server] Add(7, 3) = 10
[client] Calling Add(7, 3...)
[client] Result = 10
```
