#include <dbus/dbus.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>

// =============================================================================
// D-Bus 数据采集 — 控制客户端
// =============================================================================
// 功能：向数据采集服务发送控制指令
//   - start   → 开始采集
//   - stop    → 停止采集
//   - freq N  → 设置采样频率为 N Hz (1-100)
//   - status  → 查询采集状态
//   - quit    → 退出
//
// 所有指令通过 METHOD_CALL 发送，同步等待回复。
// 控制客户端不需要注册 Object Path 或 Service Name。
// =============================================================================

constexpr const char *DESTINATION  = "com.example.DataAcquisition";
constexpr const char *OBJECT_PATH  = "/com/example/DataAcquisition";
constexpr const char *CONTROL_IFACE = "com.example.DataAcquisition.Control";

// ---------------------------------------------------------------------------
// 通用调用辅助函数：发送 method_call 并解析 STRING 返回值
// ---------------------------------------------------------------------------
static bool call_method(DBusConnection *conn,
                        const char *method_name,
                        std::string *out_result)
{
    DBusError err;
    dbus_error_init(&err);

    DBusMessage *msg = dbus_message_new_method_call(
        DESTINATION, OBJECT_PATH, CONTROL_IFACE, method_name);
    if (!msg) return false;

    DBusMessage *reply = dbus_connection_send_with_reply_and_block(
        conn, msg, DBUS_TIMEOUT_USE_DEFAULT, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        std::cerr << "[control] " << method_name << "() failed: "
                  << err.message << std::endl;
        dbus_error_free(&err);
        return false;
    }

    if (out_result) {
        const char *s = nullptr;
        if (dbus_message_get_args(reply, &err,
                                  DBUS_TYPE_STRING, &s, DBUS_TYPE_INVALID)) {
            *out_result = s ? s : "";
        } else {
            dbus_error_free(&err);
        }
    }

    dbus_message_unref(reply);
    return true;
}

// ---------------------------------------------------------------------------
// 调用 SetFrequency(u freq)
// ---------------------------------------------------------------------------
static bool call_set_frequency(DBusConnection *conn, dbus_uint32_t freq)
{
    DBusError err;
    dbus_error_init(&err);

    DBusMessage *msg = dbus_message_new_method_call(
        DESTINATION, OBJECT_PATH, CONTROL_IFACE, "SetFrequency");
    dbus_message_append_args(msg,
                             DBUS_TYPE_UINT32, &freq,
                             DBUS_TYPE_INVALID);

    DBusMessage *reply = dbus_connection_send_with_reply_and_block(
        conn, msg, DBUS_TIMEOUT_USE_DEFAULT, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        std::cerr << "[control] SetFrequency(" << freq << ") failed: "
                  << err.message << std::endl;
        dbus_error_free(&err);
        return false;
    }

    dbus_message_unref(reply);
    std::cout << "[control] SetFrequency(" << freq << ") OK" << std::endl;
    return true;
}

// ---------------------------------------------------------------------------
// 打印帮助
// ---------------------------------------------------------------------------
static void print_help()
{
    std::cout << "\nCommands:\n"
              << "  start      — 开始采集\n"
              << "  stop       — 停止采集\n"
              << "  freq <n>   — 设置采样频率 (1-100 Hz)\n"
              << "  status     — 查询采集状态\n"
              << "  help       — 显示帮助\n"
              << "  quit       — 退出\n"
              << std::endl;
}

int main()
{
    DBusError err;
    dbus_error_init(&err);

    // 1. 连接 Session Bus
    DBusConnection *conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        std::cerr << "[control] Connection error: " << err.message << std::endl;
        dbus_error_free(&err);
        return EXIT_FAILURE;
    }

    std::cout << "[control] Data Acquisition Controller" << std::endl;
    std::cout << "[control] Connected to " << DESTINATION << std::endl;
    print_help();

    // 2. 交互式命令循环
    std::string line;
    while (true) {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        // 解析命令
        std::string cmd = line;
        std::string arg;
        auto pos = line.find(' ');
        if (pos != std::string::npos) {
            cmd = line.substr(0, pos);
            arg = line.substr(pos + 1);
        }

        if (cmd == "start") {
            std::string result;
            if (call_method(conn, "Start", &result)) {
                std::cout << "[control] Start() OK" << std::endl;
            }
        } else if (cmd == "stop") {
            std::string result;
            if (call_method(conn, "Stop", &result)) {
                std::cout << "[control] Stop() OK" << std::endl;
            }
        } else if (cmd == "freq" || cmd == "setfreq") {
            try {
                int f = std::stoi(arg);
                if (f >= 1 && f <= 100) {
                    call_set_frequency(conn, static_cast<dbus_uint32_t>(f));
                } else {
                    std::cerr << "[control] Frequency must be 1-100" << std::endl;
                }
            } catch (...) {
                std::cerr << "[control] Invalid frequency: " << arg << std::endl;
            }
        } else if (cmd == "status") {
            std::string result;
            if (call_method(conn, "GetStatus", &result)) {
                std::cout << "[control] Status: " << result << std::endl;
            }
        } else if (cmd == "help") {
            print_help();
        } else if (cmd == "quit" || cmd == "exit") {
            break;
        } else {
            std::cerr << "[control] Unknown command: " << cmd << std::endl;
            print_help();
        }
    }

    dbus_connection_unref(conn);
    std::cout << "[control] Goodbye." << std::endl;
    return EXIT_SUCCESS;
}
