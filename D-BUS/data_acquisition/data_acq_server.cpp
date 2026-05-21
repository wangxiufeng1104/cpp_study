#include <dbus/dbus.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <chrono>
#include <unistd.h>

// =============================================================================
// D-Bus 数据采集服务 — 服务端
// =============================================================================
// 功能：
//   1. 模拟传感器数据采集（正弦波 + 噪声）
//   2. 响应控制指令：Start采集 / Stop采集 / SetFrequency频率 / GetStatus状态
//   3. 按设定频率发射 DataReady 信号，向订阅者推送实时数据
//
// 架构：
//   事件循环用 20ms 超时轮询，兼顾消息分发响应速度和数据采样精度。
//   用 std::chrono 跟踪上次发射时间，实现精确的采样间隔控制。
// =============================================================================

constexpr const char *SERVICE_NAME     = "com.example.DataAcquisition";
constexpr const char *OBJECT_PATH      = "/com/example/DataAcquisition";
constexpr const char *CONTROL_IFACE    = "com.example.DataAcquisition.Control";
constexpr const char *SIGNAL_NAME      = "DataReady";

// =============================================================================
// 采集器状态
// =============================================================================
static struct {
    bool     running     = false;     // 采集开关
    uint32_t frequency   = 10;        // 采样频率 (Hz)，默认 10Hz
    double   amplitude   = 100.0;     // 信号幅值
    double   signal_freq = 2.0;       // 信号波形频率 (Hz)
} g_acq;

// 当前模拟传感器值（运行时更新）
static double   g_value     = 0.0;
static uint64_t g_timestamp = 0;

// =============================================================================
// 模拟数据生成 — 正弦波 + 随机噪声
// =============================================================================
// 用单调递增的时间戳驱动波形，保证采样间隔不均时波形仍连续。
static void generate_sample()
{
    static auto t0 = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - t0).count();

    g_value = g_acq.amplitude * std::sin(2.0 * M_PI * g_acq.signal_freq * elapsed)
              + (std::rand() % 1000 - 500) / 100.0;  // ±5 噪声

    auto us = std::chrono::duration_cast<std::chrono::microseconds>(
                  std::chrono::steady_clock::now().time_since_epoch()).count();
    g_timestamp = static_cast<uint64_t>(us);
}

// =============================================================================
// 发射 DataReady 信号
// =============================================================================
static void emit_data_signal(DBusConnection *conn)
{
    DBusMessage *signal = dbus_message_new_signal(OBJECT_PATH, CONTROL_IFACE, SIGNAL_NAME);
    if (!signal) return;

    dbus_message_append_args(signal,
                             DBUS_TYPE_DOUBLE, &g_value,
                             DBUS_TYPE_UINT64, &g_timestamp,
                             DBUS_TYPE_INVALID);

    dbus_connection_send(conn, signal, nullptr);
    dbus_message_unref(signal);
}

// =============================================================================
// method: Start() → 无参数，无返回值
// =============================================================================
static DBusMessage *handle_start(DBusMessage *msg)
{
    if (g_acq.running) {
        return dbus_message_new_error(msg,
            "com.example.DataAcquisition.Error.AlreadyRunning",
            "Acquisition is already running");
    }
    g_acq.running = true;
    std::cout << "[server] Acquisition STARTED (freq=" << g_acq.frequency
              << " Hz)" << std::endl;
    return dbus_message_new_method_return(msg);
}

// =============================================================================
// method: Stop() → 无参数，无返回值
// =============================================================================
static DBusMessage *handle_stop(DBusMessage *msg)
{
    if (!g_acq.running) {
        return dbus_message_new_error(msg,
            "com.example.DataAcquisition.Error.NotRunning",
            "Acquisition is not running");
    }
    g_acq.running = false;
    std::cout << "[server] Acquisition STOPPED" << std::endl;
    return dbus_message_new_method_return(msg);
}

// =============================================================================
// method: SetFrequency(u freq) → 无返回值
// =============================================================================
static DBusMessage *handle_set_frequency(DBusMessage *msg)
{
    DBusError err;
    dbus_error_init(&err);

    dbus_uint32_t freq = 0;
    if (!dbus_message_get_args(msg, &err,
                               DBUS_TYPE_UINT32, &freq,
                               DBUS_TYPE_INVALID)) {
        DBusMessage *err_reply = dbus_message_new_error(msg, err.name, err.message);
        dbus_error_free(&err);
        return err_reply;
    }

    if (freq < 1 || freq > 100) {
        return dbus_message_new_error(msg, DBUS_ERROR_INVALID_ARGS,
                                      "Frequency must be 1-100 Hz");
    }

    g_acq.frequency = freq;
    std::cout << "[server] Frequency set to " << freq << " Hz" << std::endl;
    return dbus_message_new_method_return(msg);
}

// =============================================================================
// method: GetStatus() → s status
// =============================================================================
static DBusMessage *handle_get_status(DBusMessage *msg)
{
    DBusMessage *reply = dbus_message_new_method_return(msg);
    const char *status = g_acq.running ? "running" : "stopped";
    dbus_message_append_args(reply,
                             DBUS_TYPE_STRING, &status,
                             DBUS_TYPE_INVALID);
    return reply;
}

// =============================================================================
// 消息分发器
// =============================================================================
static DBusHandlerResult handle_message(DBusConnection *conn,
                                        DBusMessage *msg,
                                        void * /*user_data*/)
{
    if (dbus_message_get_type(msg) != DBUS_MESSAGE_TYPE_METHOD_CALL) {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    const char *iface_name = dbus_message_get_interface(msg);
    if (!iface_name || std::strcmp(iface_name, CONTROL_IFACE) != 0) {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    const char *member = dbus_message_get_member(msg);
    if (!member) {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    DBusMessage *reply = nullptr;
    std::string m(member);

    if      (m == "Start")       reply = handle_start(msg);
    else if (m == "Stop")        reply = handle_stop(msg);
    else if (m == "SetFrequency") reply = handle_set_frequency(msg);
    else if (m == "GetStatus")   reply = handle_get_status(msg);
    else return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    if (reply) {
        dbus_connection_send(conn, reply, nullptr);
        dbus_message_unref(reply);
    }
    return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusObjectPathVTable vtable = {
    .unregister_function = nullptr,
    .message_function    = handle_message,
};

// =============================================================================
// main
// =============================================================================
int main()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    DBusError err;
    dbus_error_init(&err);

    // 1. 连接 Session Bus
    DBusConnection *conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        std::cerr << "[server] Connection error: " << err.message << std::endl;
        dbus_error_free(&err);
        return EXIT_FAILURE;
    }

    // 2. 请求 Service Name
    int ret = dbus_bus_request_name(conn, SERVICE_NAME,
                                    DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if (dbus_error_is_set(&err)) {
        std::cerr << "[server] Name error: " << err.message << std::endl;
        dbus_error_free(&err);
        return EXIT_FAILURE;
    }

    // 3. 注册 Object Path
    if (!dbus_connection_register_object_path(conn, OBJECT_PATH, &vtable, nullptr)) {
        std::cerr << "[server] Failed to register object path" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "[server] Data Acquisition Service online" << std::endl;
    std::cout << "[server]   Service: " << SERVICE_NAME << std::endl;
    std::cout << "[server]   Object:  " << OBJECT_PATH << std::endl;
    std::cout << "[server]   Methods: Start / Stop / SetFrequency / GetStatus" << std::endl;
    std::cout << "[server]   Signal:  DataReady(double, uint64)" << std::endl;
    std::cout << "[server]   Default freq: " << g_acq.frequency << " Hz" << std::endl;

    // 4. 事件循环（20ms 轮询 + 按频率发射数据）
    using clock = std::chrono::steady_clock;
    auto last_sample = clock::now();

    while (true) {
        // 20ms 超时执行消息分发（保证控制指令快速响应）
        dbus_connection_read_write_dispatch(conn, 20);

        // 如果采集已启动，按设定频率发射数据
        if (g_acq.running) {
            auto now = clock::now();
            auto interval = std::chrono::milliseconds(1000 / g_acq.frequency);
            if (now - last_sample >= interval) {
                generate_sample();
                emit_data_signal(conn);
                last_sample = now;

                std::cout << "[server] DataReady: value=" << g_value
                          << ", ts=" << g_timestamp << std::endl;
            }
        }
    }

    dbus_connection_unref(conn);
    return EXIT_SUCCESS;
}
