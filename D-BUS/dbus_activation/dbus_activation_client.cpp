#include <dbus/dbus.h>
#include <iostream>
#include <cstdlib>
#include <cstring>

// =============================================================================
// D-Bus 服务激活 (Service Activation) — 客户端
// =============================================================================
// 本客户端的关键特点：对服务是否正在运行完全不知情，也不做任何检查。
//
// 调用流程（客户端视角）：
//   1. 连接到 Session Bus
//   2. 构造 method_call("Ping") → 目标是 com.example.ActivationTest
//   3. 发送请求并阻塞等待回复
//   4. 收到回复 → 打印
//
// 调用流程（底层真实发生的事情，客户端无需感知）：
//   1. 客户端将 method_call 发送给 dbus-daemon
//   2. dbus-daemon 发现 com.example.ActivationTest 未运行
//   3. dbus-daemon 查找 .service 文件 → 读取 Exec= 指令
//   4. dbus-daemon fork+exec 启动 server 进程
//   5. Server 启动 → 连接 bus → 注册名称 → 注册对象路径
//   6. dbus-daemon 将挂起的 method_call 转发给 server
//   7. Server 处理请求 → 返回 reply → dbus-daemon 转发给客户端
//   8. 客户端收到 reply（整个过程对客户端完全透明）
//
// 这就是 D-Bus 的"按需激活"机制 — 服务对客户端透明。
// =============================================================================

constexpr const char *DESTINATION = "com.example.ActivationTest";
constexpr const char *OBJECT_PATH = "/com/example/ActivationTest";
constexpr const char *INTERFACE   = "com.example.ActivationTest";

int main()
{
    DBusError err;
    dbus_error_init(&err);

    // 1. 连接到 Session Bus
    DBusConnection *conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        std::cerr << "[client] Connection error: "
                  << err.message << std::endl;
        dbus_error_free(&err);
        return EXIT_FAILURE;
    }

    std::cout << "[client] Calling Ping() on "
              << DESTINATION << " ..." << std::endl;
    std::cout << "[client]   (If the service is not running, "
              << "dbus-daemon will auto-start it.)" << std::endl;

    // 2. 构造方法调用消息
    // 注意：客户端完全不检查服务是否存在 —
    //       如果 server 未运行，dbus-daemon 会自动启动它
    DBusMessage *msg = dbus_message_new_method_call(
        DESTINATION, OBJECT_PATH, INTERFACE, "Ping");
    if (!msg) {
        std::cerr << "[client] Failed to create method call" << std::endl;
        return EXIT_FAILURE;
    }

    // 3. 同步发送并阻塞等待回复
    // ============================================
    // send_with_reply_and_block 会一直阻塞直到：
    //   - 收到 METHOD_RETURN → 返回 reply
    //   - 收到 ERROR → err 被设置
    //   - 超时 → err 被设置为 DBUS_ERROR_TIMEOUT
    //
    // 如果 server 未运行且没有 .service 文件：
    //   err 会被设置为 "The name com.example.ActivationTest
    //   was not provided by any .service files"
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(
        conn, msg, DBUS_TIMEOUT_USE_DEFAULT, &err);
    dbus_message_unref(msg);

    // 检查调用是否失败
    if (dbus_error_is_set(&err)) {
        std::cerr << "[client] Call failed: "
                  << err.message << std::endl;
        dbus_error_free(&err);
        return EXIT_FAILURE;
    }

    // 4. 解析回复：提取 STRING 返回值
    const char *pong = nullptr;
    if (dbus_message_get_args(reply, &err,
                              DBUS_TYPE_STRING, &pong,
                              DBUS_TYPE_INVALID)) {
        std::cout << "[client] Reply: " << (pong ? pong : "(null)")
                  << std::endl;
    } else {
        std::cerr << "[client] Parse error: "
                  << err.message << std::endl;
        dbus_error_free(&err);
    }

    // 5. 清理
    dbus_message_unref(reply);
    dbus_connection_unref(conn);
    return EXIT_SUCCESS;
}
