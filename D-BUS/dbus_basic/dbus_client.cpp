#include <dbus/dbus.h>
#include <iostream>
#include <cstdlib>

// =============================================================================
// D-Bus 目标定位（必须与 server 端完全一致）
// =============================================================================
// DESTINATION — 目标 Service Name，daemon 根据此字段将消息路由到 server 进程
// OBJECT_PATH — 目标对象路径，server 在注册 vtable 时使用的路径
// INTERFACE   — 目标接口名，server handler 匹配接口时使用的名称
constexpr const char *DESTINATION = "com.example.DBusTutorial";
constexpr const char *OBJECT_PATH = "/com/example/Tutorial";
constexpr const char *INTERFACE   = "com.example.DBusTutorial";

int main()
{
    DBusError err;
    dbus_error_init(&err);

    // 1. 连接到 Session Bus
    // 客户端只需要连接，不需要请求 Service Name
    DBusConnection *conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        std::cerr << "[client] Connection error: "
                  << err.message << std::endl;
        dbus_error_free(&err);
        return EXIT_FAILURE;
    }
    if (!conn) {
        std::cerr << "[client] dbus_bus_get returned null" << std::endl;
        return EXIT_FAILURE;
    }

    // 2. 构造方法调用消息（METHOD_CALL）
    // 参数依次为：目标服务名、对象路径、接口名、方法名
    DBusMessage *msg = dbus_message_new_method_call(
        DESTINATION, OBJECT_PATH, INTERFACE, "Add");
    if (!msg) {
        std::cerr << "[client] Failed to create method call" << std::endl;
        return EXIT_FAILURE;
    }

    // 3. 向消息中追加方法参数
    // 参数按类型顺序附加：int32 a, int32 b
    // DBUS_TYPE_INVALID 标记参数列表结束
    dbus_int32_t a = 7, b = 3;
    dbus_message_append_args(msg,
                             DBUS_TYPE_INT32, &a,
                             DBUS_TYPE_INT32, &b,
                             DBUS_TYPE_INVALID);

    std::cout << "[client] Calling Add(" << a << ", " << b << "...)"
              << std::endl;

    // 4. 同步发送请求并阻塞等待回复
    // dbus_connection_send_with_reply_and_block 是同步 RPC 调用：
    //   - 发送 METHOD_CALL 消息
    //   - 阻塞当前线程，等待 METHOD_RETURN 或 ERROR
    //   - DBUS_TIMEOUT_USE_DEFAULT 使用默认超时（通常 25 秒）
    // 注意：发送后 msg 可以立即释放，不影响等待中的请求
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(
        conn, msg, DBUS_TIMEOUT_USE_DEFAULT, &err);

    // 消息已发送，立即释放请求消息引用
    dbus_message_unref(msg);

    // 检查调用是否出错（server 可能返回 ERROR 消息）
    if (dbus_error_is_set(&err)) {
        std::cerr << "[client] Call failed: "
                  << err.message << std::endl;
        dbus_error_free(&err);
        return EXIT_FAILURE;
    }

    // 5. 解析回复消息中的返回值
    dbus_int32_t result = 0;
    if (!dbus_message_get_args(reply, &err,
                               DBUS_TYPE_INT32, &result,
                               DBUS_TYPE_INVALID)) {
        std::cerr << "[client] Failed to parse reply: "
                  << err.message << std::endl;
        dbus_error_free(&err);
        dbus_message_unref(reply);
        return EXIT_FAILURE;
    }

    std::cout << "[client] Result = " << result << std::endl;

    // 6. 清理资源（释放引用计数）
    dbus_message_unref(reply);
    dbus_connection_unref(conn);
    return EXIT_SUCCESS;
}
