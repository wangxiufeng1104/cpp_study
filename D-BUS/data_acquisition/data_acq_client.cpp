#include <dbus/dbus.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <iomanip>

// =============================================================================
// D-Bus 数据采集 — 数据接收客户端
// =============================================================================
// 功能：
//   1. 通过 match rule 订阅 DataReady 信号
//   2. 进入事件循环，接收并实时打印采集数据
//   3. 按 Ctrl+C 退出
//
// 设计要点：
//   使用 read_write + pop_message 模式（而非 vtable dispatch），
//   因为数据客户端只需要被动接收信号，不需要响应方法调用。
//   50ms 轮询间隔兼顾实时性和 CPU 占用。
//
// 数据展示：
//   每秒统计一次接收到的数据点数，用于验证采样频率是否正确。
// =============================================================================

constexpr const char *OBJECT_PATH  = "/com/example/DataAcquisition";
constexpr const char *CONTROL_IFACE = "com.example.DataAcquisition.Control";
constexpr const char *SIGNAL_NAME  = "DataReady";

int main()
{
    DBusError err;
    dbus_error_init(&err);

    // 1. 连接到 Session Bus
    DBusConnection *conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        std::cerr << "[client] Connection error: " << err.message << std::endl;
        dbus_error_free(&err);
        return EXIT_FAILURE;
    }

    // 2. 注册 match rule 订阅 DataReady 信号
    // ============================================
    // 订阅 com.example.DataAcquisition.Control.DataReady 信号，
    // 不限定 sender（可以是任意进程发送的此信号）。
    const char *rule =
        "type='signal',"
        "interface='com.example.DataAcquisition.Control',"
        "member='DataReady'";
    dbus_bus_add_match(conn, rule, &err);
    dbus_connection_flush(conn);

    if (dbus_error_is_set(&err)) {
        std::cerr << "[client] Match error: " << err.message << std::endl;
        dbus_error_free(&err);
        return EXIT_FAILURE;
    }

    std::cout << "[client] Listening for DataReady signals..." << std::endl;
    std::cout << "[client] Press Ctrl+C to exit." << std::endl;
    std::cout << std::fixed << std::setprecision(3);

    // 统计变量：用于验证采样频率
    long        received   = 0;
    double      latest     = 0.0;
    uint64_t    latest_ts  = 0;
    auto        report_at  = std::chrono::steady_clock::now();
    long        batch      = 0;

    // 3. 事件循环
    while (true) {
        // 50ms 超时，兼顾实时性和 CPU 占用
        dbus_connection_read_write(conn, 50);

        DBusMessage *msg = nullptr;
        while ((msg = dbus_connection_pop_message(conn)) != nullptr) {

            // 验证是否为 DataReady 信号
            if (!dbus_message_is_signal(msg, CONTROL_IFACE, SIGNAL_NAME)) {
                dbus_message_unref(msg);
                continue;
            }

            // 解析信号参数：double value, uint64 timestamp
            double        val = 0.0;
            dbus_uint64_t ts  = 0;
            if (dbus_message_get_args(msg, &err,
                                      DBUS_TYPE_DOUBLE, &val,
                                      DBUS_TYPE_UINT64, &ts,
                                      DBUS_TYPE_INVALID)) {
                latest    = val;
                latest_ts = ts;
                ++received;
                ++batch;

                // 每 10 个数据点打印一次（高频时不刷屏）
                if (received % 10 == 0) {
                    std::cout << "[client] #" << received
                              << "  val=" << std::setw(12) << val
                              << "  ts=" << ts << std::endl;
                }
            } else {
                std::cerr << "[client] Parse error: " << err.message << std::endl;
                dbus_error_free(&err);
                dbus_error_init(&err);
            }

            dbus_message_unref(msg);
        }

        // 每秒输出一次速率统计
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration<double>(now - report_at).count() >= 1.0) {
            std::cout << "[client] ── rate: " << batch << " samples/sec"
                      << " | total=" << received << std::endl;
            batch = 0;
            report_at = now;
        }
    }

    dbus_bus_remove_match(conn, rule, &err);
    dbus_connection_unref(conn);
    return EXIT_SUCCESS;
}
