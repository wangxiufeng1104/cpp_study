# D-Bus 服务激活 (Activation)

演示 D-Bus daemon 的**按需启动** (on-demand activation) 机制：
客户端向未运行的服务发送消息时，dbus-daemon 自动启动服务进程。

## 核心概念

- **服务激活**: 当客户端向某个 Service Name 发送请求时，如果该服务未运行，dbus-daemon
  会根据 `.service` 文件中的 `Exec=` 指令自动启动对应的可执行文件
- **Service 文件**: 放置在特定目录的 ini 格式文件，告诉 dbus-daemon 如何启动服务
- **激活流程**: Client → dbus-daemon (name lookup) → 启动 server → 建立连接
  → 转发消息 → 返回 reply
- **FLAG_ALLOW_REPLACEMENT**: 服务端请求名称时使用此 flag，允许其他进程替换
- **透明性**: 客户端代码无需感知服务是否正在运行 — D-Bus 对调用者透明

## 安装 Service 文件

### 方式 1: 用户级别（推荐）
```bash
mkdir -p ~/.local/share/dbus-1/services
cp com.example.ActivationTest.service ~/.local/share/dbus-1/services/
```

### 方式 2: 系统级别（需 root）
```bash
sudo cp com.example.ActivationTest.service /usr/share/dbus-1/services/
```

> 注意：需要将 `.service` 文件中的 `@BUILDDIR@` 替换为实际的 build 目录路径：
> ```bash
> sed -i "s|@BUILDDIR@|$(pwd)|g" ~/.local/share/dbus-1/services/com.example.ActivationTest.service
> ```

## 运行

```bash
# 1. 安装 service 文件（仅一次）
mkdir -p ~/.local/share/dbus-1/services
sed "s|@BUILDDIR@|$(realpath .)|g" \
  dbus_activation/com.example.ActivationTest.service \
  > ~/.local/share/dbus-1/services/com.example.ActivationTest.service

# 2. 直接运行客户端（无需先启动服务端）
./bin/dbus_activation_client
```

## 输出示例

```
[client] Calling Ping() on com.example.ActivationTest ...
[client]   (If the service is not running, dbus-daemon will auto-start it.)
[server] Activated! Listening on com.example.ActivationTest at /com/example/ActivationTest
[server] Ping received
[server]   sender: :1.123
[client] Reply: Pong — service was activated by dbus-daemon
[server] Idle timeout. Exiting.
```
