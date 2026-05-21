#include <dbus/dbus.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <ctime>

// =============================================================================
// D-Bus 信号三大定位要素
// =============================================================================
// 与 method_call 不同，信号不需要 destination（服务名）。
// 信号通过 path + interface + name 三元组标识，daemon 根据 match rule 广播。
constexpr const char *SIGNAL_PATH      = "/com/example/Tutorial";
constexpr const char *SIGNAL_INTERFACE = "com.example.DBusTutorial";
constexpr const char *SIGNAL_NAME      = "ChatMessage";

int main()
{
    DBusError err;
    dbus_error_init(&err);

    // 1. 连接到 Session Bus
    // 注意：信号发送者不需要请求 Service Name
    // 只需要一个连接即可发送信号（daemon 会自动分配 :1.xx 连接名）
    DBusConnection *conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        std::cerr << "[sender] Connection error: "
                  << err.message << std::endl;
        dbus_error_free(&err);
        return EXIT_FAILURE;
    }

    // 准备随机消息池（演示用）
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    const char *messages[] = {
        "Hello from D-Bus!",
        "Signals are broadcast to all listeners.",
        "No reply needed for signals.",
        "D-Bus decouples sender and receiver.",
    };
    const int msg_count = sizeof(messages) / sizeof(messages[0]);

    std::cout << "[sender] Sending signals to interface "
              << SIGNAL_INTERFACE << " ..." << std::endl;

    // 2. 循环发送 8 条信号
    for (int i = 0; i < 8; ++i) {
        // dbus_message_new_signal 创建 SIGNAL 类型消息
        // 参数：(对象路径, 接口名, 信号名)
        // 与 method_call 不同：信号没有 destination 字段（广播语义）
        DBusMessage *signal = dbus_message_new_signal(
            SIGNAL_PATH, SIGNAL_INTERFACE, SIGNAL_NAME);
        if (!signal) continue;

        const char *sender_name = "SignalBot";
        const char *text        = messages[std::rand() % msg_count];
        dbus_int32_t seq        = i + 1;

        // 将信号参数附加到消息中：string sender_name, string text, int32 seq
        dbus_message_append_args(signal,
                                 DBUS_TYPE_STRING, &sender_name,
                                 DBUS_TYPE_STRING, &text,
                                 DBUS_TYPE_INT32,  &seq,
                                 DBUS_TYPE_INVALID);

        // 3. 发送信号（fire-and-forget 模式）
        // 信号不需要等待回复 — 发送后立即返回
        // daemon 负责将信号推送给所有匹配 match rule 的订阅者
        dbus_connection_send(conn, signal, nullptr);

        // 信号已提交到发送缓冲区，立即释放引用
        dbus_message_unref(signal);

        std::cout << "[sender] #" << seq << " " << sender_name
                  << ": " << text << std::endl;

        // 间隔 0.5 秒，便于观察 receiver 的实时接收效果
        usleep(500000);
    }

    // 4. 确保缓冲区中的信号全部发出
    // dbus_connection_flush 阻塞直到待发送数据全部写入 socket
    // 如果不调用 flush，消息可能还停留在 libdbus 内部缓冲区
    dbus_connection_flush(conn);

    // 额外等待 0.2 秒，给 receiver 的 dbus_connection_read_write 留出处理时间
    usleep(200000);

    dbus_connection_unref(conn);
    std::cout << "[sender] Done." << std::endl;
    return EXIT_SUCCESS;
}
