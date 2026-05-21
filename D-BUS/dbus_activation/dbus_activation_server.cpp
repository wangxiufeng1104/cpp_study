#include <dbus/dbus.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

// =============================================================================
// D-Bus 服务激活 (Service Activation) — 服务端
// =============================================================================
// 服务激活是 D-Bus 的核心机制之一：
//   1. 客户端向一个未运行的 Service Name 发送消息
//   2. dbus-daemon 查找对应的 .service 文件
//   3. dbus-daemon fork+exec 启动本服务进程
//   4. 本进程启动 → 连接 bus → 请求 Service Name → 注册 Object Path
//   5. dbus-daemon 将客户端请求转发给本进程
//   6. 本进程处理请求并回复
//
// 整个过程对客户端完全透明 — 客户端不需要检查服务是否在运行。
// 这是 D-Bus 的"按需启动" (on-demand activation) 特性。
//
// 本 server 的特殊设计：
//   - 使用空闲超时退出机制（idle timeout），退出后可以被 dbus-daemon 再次激活
//   - 这模拟了真实服务的"按需运行"模式（如 systemd socket activation）
// =============================================================================

constexpr const char *SERVICE_NAME = "com.example.ActivationTest";
constexpr const char *OBJECT_PATH  = "/com/example/ActivationTest";
constexpr const char *INTERFACE    = "com.example.ActivationTest";

// =============================================================================
// D-Bus 消息处理器 — 实现 Ping 方法
// =============================================================================
// 业务逻辑非常简单：收到 Ping，返回 Pong。
// 关键学习点：dbus_message_get_sender 可以获取调用方的连接标识。
static DBusHandlerResult handle_message(DBusConnection *conn,
                                        DBusMessage *msg,
                                        void * /*user_data*/)
{
    if (!dbus_message_is_method_call(msg, INTERFACE, "Ping")) {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    std::cout << "[server] Ping received" << std::endl;

    // dbus_message_get_sender 返回消息发送者的唯一连接名
    // 服务激活场景下，这个 sender 是客户端连接名（如 :1.42）
    const char *sender = dbus_message_get_sender(msg);
    std::cout << "[server]   sender: "
              << (sender ? sender : "(null)") << std::endl;

    // 构造回复
    DBusMessage *reply = dbus_message_new_method_return(msg);
    const char *pong = "Pong — service was activated by dbus-daemon";
    dbus_message_append_args(reply,
                             DBUS_TYPE_STRING, &pong,
                             DBUS_TYPE_INVALID);
    dbus_connection_send(conn, reply, nullptr);
    dbus_message_unref(reply);

    return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusObjectPathVTable vtable = {
    .unregister_function = nullptr,
    .message_function    = handle_message,
};

int main()
{
    DBusError err;
    dbus_error_init(&err);

    // 1. 连接到 Session Bus
    DBusConnection *conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        std::cerr << "[server] Connection error: "
                  << err.message << std::endl;
        dbus_error_free(&err);
        return EXIT_FAILURE;
    }

    // 2. 请求 Service Name
    // ============================================
    // DBUS_NAME_FLAG_REPLACE_EXISTING  — 如果名称已被旧进程占用，替换它
    // DBUS_NAME_FLAG_ALLOW_REPLACEMENT — 允许未来的新进程替换本进程
    //
    // 两个 flag 配合使用：
    //   - 首次启动：正常获取名称
    //   - 旧进程未退出时 dbus-daemon 尝试重启本服务：新进程可以替换旧进程的名称
    //   - 这是服务激活"无缝重启"的基础
    int ret = dbus_bus_request_name(conn, SERVICE_NAME,
                                    DBUS_NAME_FLAG_REPLACE_EXISTING
                                    | DBUS_NAME_FLAG_ALLOW_REPLACEMENT,
                                    &err);
    if (dbus_error_is_set(&err)) {
        std::cerr << "[server] Name error: "
                  << err.message << std::endl;
        dbus_error_free(&err);
        return EXIT_FAILURE;
    }

    // 3. 注册对象路径
    if (!dbus_connection_register_object_path(conn, OBJECT_PATH,
                                               &vtable, nullptr)) {
        std::cerr << "[server] Failed to register object path"
                  << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "[server] Activated! Listening on "
              << SERVICE_NAME << " at " << OBJECT_PATH << std::endl;

    // 4. 空闲超时事件循环（演示"按需运行 → 自动退出"模式）
    // ============================================
    // 真实服务通常配合 systemd socket activation 使用：
    //   - 有请求时 systemd 启动服务
    //   - 服务空闲一段时间后自动退出
    //   - 下次有请求时 systemd 再次启动
    //
    // 本示例模拟这一模式：
    //   - read_write_dispatch 参数 100ms 表示最多等 100ms
    //   - 如果有消息到达（返回 TRUE），重置空闲计数器
    //   - 如果超时无消息（返回 FALSE），空闲计数器 +1
    //   - 累计 300 次（约 30 秒）无消息后退出
    //   - 退出后，dbus-daemon 可以在下次有请求时再次激活本服务
    int idle_count = 0;
    while (idle_count < 300) {
        if (dbus_connection_read_write_dispatch(conn, 100)) {
            idle_count = 0;          // 有活动，重置空闲计数器
        } else {
            ++idle_count;            // 无活动，递增
        }
    }

    std::cout << "[server] Idle timeout. Exiting." << std::endl;
    dbus_connection_unref(conn);
    return EXIT_SUCCESS;
}
