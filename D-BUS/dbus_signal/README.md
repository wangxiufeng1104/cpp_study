# D-Bus 信号 (Signal) — 发布/订阅

演示 D-Bus 的广播机制：发送者发布信号，多个接收者可以同时监听。

## 核心概念

- **Signal**: `dbus_message_new_signal(path, iface, name)` 创建信号消息
- **发送**: `dbus_connection_send()` 发送信号，无需等待回复
- **Match Rule**: 字符串规则 `type='signal',interface='...',member='...'` 过滤感兴趣的信号
- **订阅**: `dbus_bus_add_match()` 向 daemon 注册 match rule
- **接收**: `dbus_connection_pop_message()` + `dbus_message_is_signal()` 从连接中取出信号
- **取消订阅**: `dbus_bus_remove_match()` 移除 match rule

## 运行

```bash
# 终端 1: 启动接收者
./bin/dbus_signal_receiver

# 终端 2: 发送信号
./bin/dbus_signal_sender
```

> 可以启动多个 receiver 同时运行，观察信号的广播特性。

## 输出示例

```
[sender] Sending signals to interface com.example.DBusTutorial ...
[sender] #1 SignalBot: D-Bus decouples sender and receiver.
[sender] #2 SignalBot: No reply needed for signals.
...
[receiver] Listening for signals on interface com.example.DBusTutorial ...
[receiver] #1 SignalBot: D-Bus decouples sender and receiver.
[receiver] #2 SignalBot: No reply needed for signals.
```
