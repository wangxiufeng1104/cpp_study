#include <dbus/dbus.h>
#include <iostream>
#include <cstdlib>
#include <cstring>

// =============================================================================
// D-Bus 信号订阅定位（必须与 sender 完全一致）
// =============================================================================
// 接收者通过 match rule 告诉 daemon"我对哪些信号感兴趣"。
// match rule 使用 path + interface + name 三元组过滤，与 sender 的发送参数匹配。
constexpr const char *SIGNAL_PATH      = "/com/example/Tutorial";
constexpr const char *SIGNAL_INTERFACE = "com.example.DBusTutorial";
constexpr const char *SIGNAL_NAME      = "ChatMessage";

int main()
{
    DBusError err;
    dbus_error_init(&err);

    // 1. 连接到 Session Bus
    // 接收者只需要连接，不需要请求 Service Name 或注册 Object Path
    DBusConnection *conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        std::cerr << "[receiver] Connection error: "
                  << err.message << std::endl;
        dbus_error_free(&err);
        return EXIT_FAILURE;
    }

    // 2. 向 daemon 注册 match rule（信号订阅）
    // ============================================
    // match rule 是字符串格式的过滤规则，告诉 daemon 哪些信号要转发给本连接。
    // 这是一种"服务端过滤"机制 — 不匹配的信号根本不会发送到本进程，
    // 避免了不必要的 IPC 开销。
    //
    // match rule 参数说明：
    //   type='signal'                        — 只匹配信号消息（排除 method_call/reply/error）
    //   interface='com.example.DBusTutorial'  — 过滤接口名
    //   member='ChatMessage'                 — 过滤信号名（member 即信号/方法名）
    //
    // 更多可选参数：sender, path, path_namespace, destination 等
    const char *rule =
        "type='signal',"
        "interface='com.example.DBusTutorial',"
        "member='ChatMessage'";

    // dbus_bus_add_match 将 match rule 发送给 daemon
    // daemon 记录后，从此所有匹配的信号都会转发给本连接
    dbus_bus_add_match(conn, rule, &err);

    // dbus_connection_flush 确保 match rule 已写入 socket 并被 daemon 处理
    dbus_connection_flush(conn);

    if (dbus_error_is_set(&err)) {
        std::cerr << "[receiver] Match error: "
                  << err.message << std::endl;
        dbus_error_free(&err);
        return EXIT_FAILURE;
    }

    std::cout << "[receiver] Listening for signals on interface "
              << SIGNAL_INTERFACE << " ..." << std::endl;

    // 3. 事件循环：接收信号直到收到 4 条
    // ========================================
    // 使用 dbus_connection_read_write + dbus_connection_pop_message 模式，
    // 而不是 dbus_connection_read_write_dispatch。
    //
    // 区别：
    //   read_write_dispatch — 自动将消息分发给已注册的 vtable handler
    //   read_write + pop_message — 手动从队列中逐条取出消息处理
    //
    // 信号接收场景下通常使用 pop_message 模式，因为：
    //   - 信号不需要"回复"，不需要 handler 模式
    //   - 可以灵活控制消息处理时机
    int received = 0;
    while (received < 4) {
        // dbus_connection_read_write 执行两步：
        //   read  — 从 socket 读取数据到内部缓冲区
        //   write — 将待发送数据写入 socket
        // 参数 10000 表示最多等待 10 秒（超时返回，不阻塞永久）
        // ===========================================
        // 注意：read_write 不执行 dispatch！
        // 消息只是被读入缓冲区，需要通过 pop_message 手动取出。
        // 这与 server 端的 read_write_dispatch（自动分发）不同。
        dbus_connection_read_write(conn, 10000);

        DBusMessage *msg = nullptr;

        // dbus_connection_pop_message 从接收队列头部取出一条消息
        // 返回 nullptr 表示队列为空（超时或暂无消息）
        // 循环取出所有缓冲消息，逐个处理
        while ((msg = dbus_connection_pop_message(conn)) != nullptr) {

            // 4. 验证消息类型：确保是目标信号
            // ================================
            // daemon 已经根据 match rule 过滤了信号，理论上到达这里的都是匹配的消息。
            // 但保险起见（可能还有其他 handler 注册了其他 match rule），
            // 仍然用 dbus_message_is_signal 做一次客户端验证。
            if (!dbus_message_is_signal(msg,
                                        SIGNAL_INTERFACE,
                                        SIGNAL_NAME)) {
                dbus_message_unref(msg);
                continue;
            }

            // 5. 解析信号参数：string sender, string text, int32 seq
            const char *sender_name = nullptr;
            const char *text        = nullptr;
            dbus_int32_t seq        = 0;

            if (dbus_message_get_args(msg, &err,
                                      DBUS_TYPE_STRING, &sender_name,
                                      DBUS_TYPE_STRING, &text,
                                      DBUS_TYPE_INT32,  &seq,
                                      DBUS_TYPE_INVALID)) {
                std::cout << "[receiver] #" << seq << " "
                          << sender_name << ": " << text << std::endl;
                ++received;
            } else {
                std::cerr << "[receiver] Parse error: "
                          << err.message << std::endl;
                // 解析失败时清空 error 对象，以便下次使用
                dbus_error_free(&err);
                dbus_error_init(&err);
            }

            // 每条消息处理完后释放引用
            dbus_message_unref(msg);
        }
    }

    // 6. 清理：移除 match rule 并断开连接
    // dbus_bus_remove_match 通知 daemon 停止转发匹配的信号
    dbus_bus_remove_match(conn, rule, &err);
    dbus_connection_unref(conn);
    std::cout << "[receiver] Done." << std::endl;
    return EXIT_SUCCESS;
}
