#include <dbus/dbus.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>

// =============================================================================
// D-Bus 标准 Properties 接口 — 客户端实现
// =============================================================================
// 演示如何调用 org.freedesktop.DBus.Properties 的 Get / Set / GetAll 方法。
//
// 本示例演示的核心知识点：
//   - VARIANT 类型的反序列化（recurse → get_arg_type → get_basic）
//   - VARIANT 类型的序列化（open_container → append_basic → close_container）
//   - ARRAY of DICT_ENTRY 的遍历（recurse → next 循环）
//   - 何时必须用 DBusMessageIter（构造/解析复合类型时必须）
//   - 何时可用 dbus_message_append_args（简单基本类型时更简洁）
// =============================================================================

constexpr const char *DESTINATION = "com.example.DBusTutorial";
constexpr const char *OBJECT_PATH = "/com/example/Tutorial";
constexpr const char *PROP_IFACE  = "com.example.DBusTutorial.Properties";

// =============================================================================
// call_get — 调用 Properties.Get(STRING iface, STRING prop) → 解析 VARIANT 回复
// =============================================================================
// Get 的返回值是 VARIANT（即"运行时确定类型的值"）。
// 需要先用 recurse 进入 VARIANT 内部，再用 get_arg_type 判断实际类型。
static void call_get(DBusConnection *conn,
                     const std::string &prop_name)
{
    DBusError err;
    dbus_error_init(&err);

    // 构造方法调用：目标=org.freedesktop.DBus.Properties，方法=Get
    DBusMessage *msg = dbus_message_new_method_call(
        DESTINATION, OBJECT_PATH,
        "org.freedesktop.DBus.Properties", "Get");

    // 两个 STRING 参数：接口名、属性名
    const char *iface = PROP_IFACE;
    const char *prop  = prop_name.c_str();
    dbus_message_append_args(msg,
                             DBUS_TYPE_STRING, &iface,
                             DBUS_TYPE_STRING, &prop,
                             DBUS_TYPE_INVALID);

    // 同步调用
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(
        conn, msg, DBUS_TIMEOUT_USE_DEFAULT, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        std::cerr << "[client] Get(" << prop_name << ") failed: "
                  << err.message << std::endl;
        dbus_error_free(&err);
        return;
    }

    // 解析回复：Get 的返回值是一个 VARIANT
    DBusMessageIter iter;
    if (!dbus_message_iter_init(reply, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT) {
        std::cerr << "[client] Get(" << prop_name
                  << ") unexpected reply type" << std::endl;
        dbus_message_unref(reply);
        return;
    }

    // recurse 进入 VARIANT 容器，获取内部实际类型和值
    DBusMessageIter sub;
    dbus_message_iter_recurse(&iter, &sub);
    int val_type = dbus_message_iter_get_arg_type(&sub);  // 运行时类型

    std::cout << "[client] Get(" << prop_name << ") = ";
    if (val_type == DBUS_TYPE_BOOLEAN) {
        dbus_bool_t v = FALSE;
        dbus_message_iter_get_basic(&sub, &v);
        std::cout << (v ? "true" : "false") << " (bool)";
    } else if (val_type == DBUS_TYPE_INT32) {
        dbus_int32_t v = 0;
        dbus_message_iter_get_basic(&sub, &v);
        std::cout << v << " (int32)";
    } else {
        std::cout << "<unknown type:" << val_type << ">";
    }
    std::cout << std::endl;

    dbus_message_unref(reply);
}

// =============================================================================
// call_set — 调用 Properties.Set(STRING iface, STRING prop, VARIANT value)
// =============================================================================
// Set 的参数包含 VARIANT，不能直接使用 dbus_message_append_args
// （因为它无法表达"运行时带类型标签的值"）。
// 必须使用 DBusMessageIter 手动构造消息体。
static void call_set(DBusConnection *conn,
                     const std::string &prop_name,
                     bool value)
{
    DBusError err;
    dbus_error_init(&err);

    // 构造方法调用消息
    DBusMessage *msg = dbus_message_new_method_call(
        DESTINATION, OBJECT_PATH,
        "org.freedesktop.DBus.Properties", "Set");

    // ===================================================================
    // 用 DBusMessageIter 逐参数构造消息体
    // （Set 的参数包含 VARIANT 复合类型，不能使用 dbus_message_append_args）
    // ===================================================================
    DBusMessageIter iter, sub;
    dbus_message_iter_init_append(msg, &iter);

    const char *iface = PROP_IFACE;
    const char *prop  = prop_name.c_str();

    // arg 0: STRING — 接口名
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &iface);

    // arg 1: STRING — 属性名
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &prop);

    // arg 2: VARIANT(bool) — 属性值
    // 步骤：open_container → append_basic → close_container
    dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT,
                                     DBUS_TYPE_BOOLEAN_AS_STRING, &sub);
    dbus_bool_t dbus_val = value ? TRUE : FALSE;
    dbus_message_iter_append_basic(&sub, DBUS_TYPE_BOOLEAN, &dbus_val);
    dbus_message_iter_close_container(&iter, &sub);

    // 同步发送并等待回复
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(
        conn, msg, DBUS_TIMEOUT_USE_DEFAULT, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        std::cerr << "[client] Set(" << prop_name << ", "
                  << (value ? "true" : "false") << ") failed: "
                  << err.message << std::endl;
        dbus_error_free(&err);
        return;
    }

    std::cout << "[client] Set(" << prop_name << ", "
              << (value ? "true" : "false") << ") OK" << std::endl;
    dbus_message_unref(reply);
}

// =============================================================================
// call_get_all — 调用 Properties.GetAll(STRING iface) → 解析 ARRAY<DICT<STRING,VARIANT>>
// =============================================================================
// GetAll 的返回值是 ARRAY of DICT_ENTRY{STRING, VARIANT}
//
// 【如何获取 server 的 dict？】
//   Server 端在 handle_get_all 中构造了一个 D-Bus 数组 dict，
//   用 open_container/append_basic/close_container 逐属性打包。
//   这个数组通过 D-Bus wire protocol 序列化后传回客户端。
//   客户端用 dbus_message_iter 反序列化 → recurse 进入数组 → 逐个条目取出。
//
// 【如何知道 server 有几个属性？】
//   客户端不需要提前知道属性数量。
//   D-Bus 数组在 wire protocol 中是"长度前缀"格式 — 数组头部记录了总字节数。
//   客户端通过 dbus_message_iter_recurse 获得子迭代器后，
//   只需循环调用 get_arg_type() 并判断返回值：
//     DBUS_TYPE_INVALID → 数组已遍历完，停止
//     其他类型         → 还有条目，继续
//   用 dbus_message_iter_next() 推进到下一个条目。
//   这个 while 循环天然适配任意数量的属性，server 增减属性无需修改客户端。
//
// 解析步骤：
//   1. init 顶层迭代器，验证顶层类型 = ARRAY
//   2. recurse 进入 ARRAY 子迭代器
//   3. while 循环遍历 ARRAY 中的每个 DICT_ENTRY，直到 DBUS_TYPE_INVALID
//   4.   对每个 DICT_ENTRY：recurse → get_basic(key) → next → recurse → get_basic(value)
//   5.   对每个 VARIANT value：recurse → get_arg_type → get_basic
static void call_get_all(DBusConnection *conn)
{
    DBusError err;
    dbus_error_init(&err);

    // 构造方法调用
    DBusMessage *msg = dbus_message_new_method_call(
        DESTINATION, OBJECT_PATH,
        "org.freedesktop.DBus.Properties", "GetAll");
    const char *iface = PROP_IFACE;
    dbus_message_append_args(msg,
                             DBUS_TYPE_STRING, &iface,
                             DBUS_TYPE_INVALID);

    DBusMessage *reply = dbus_connection_send_with_reply_and_block(
        conn, msg, DBUS_TIMEOUT_USE_DEFAULT, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        std::cerr << "[client] GetAll failed: "
                  << err.message << std::endl;
        dbus_error_free(&err);
        return;
    }

    // 验证顶层类型 = ARRAY
    DBusMessageIter iter;
    if (!dbus_message_iter_init(reply, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY) {
        std::cerr << "[client] GetAll unexpected reply type"
                  << std::endl;
        dbus_message_unref(reply);
        return;
    }

    std::cout << "[client] GetAll():" << std::endl;

    // recurse 进入 ARRAY，获得遍历子迭代器
    DBusMessageIter arr;
    dbus_message_iter_recurse(&iter, &arr);

    // ================================================================
    // 遍历数组：无需知道元素个数，直到 DBUS_TYPE_INVALID 停止
    // ================================================================
    // D-Bus 数组在 wire 层自带长度信息，反序列化后迭代器自动感知边界。
    // arr 初始指向第一个元素，每执行一次 dbus_message_iter_next 前进一个。
    // 当 arr 越过最后一个元素时，get_arg_type 返回 DBUS_TYPE_INVALID。
    while (dbus_message_iter_get_arg_type(&arr) != DBUS_TYPE_INVALID) {
        // 当前条目是一个 DICT_ENTRY{STRING, VARIANT}
        DBusMessageIter entry;
        dbus_message_iter_recurse(&arr, &entry);

        // DICT_ENTRY 包含两个字段：key(STRING) + value(VARIANT)

        // --- key: STRING ---
        const char *key = nullptr;
        dbus_message_iter_get_basic(&entry, &key);

        // --- value: VARIANT ---
        // dbus_message_iter_next 将 entry 推进到 DICT_ENTRY 的第二字段
        dbus_message_iter_next(&entry);
        if (dbus_message_iter_get_arg_type(&entry) == DBUS_TYPE_VARIANT) {
            // recurse 进入 VARIANT，获取内部实际类型和值
            DBusMessageIter sub;
            dbus_message_iter_recurse(&entry, &sub);
            int val_type = dbus_message_iter_get_arg_type(&sub);

            std::cout << "  " << (key ? key : "?") << " = ";
            if (val_type == DBUS_TYPE_BOOLEAN) {
                dbus_bool_t v = FALSE;
                dbus_message_iter_get_basic(&sub, &v);
                std::cout << (v ? "true" : "false");
            } else if (val_type == DBUS_TYPE_INT32) {
                dbus_int32_t v = 0;
                dbus_message_iter_get_basic(&sub, &v);
                std::cout << v;
            }
            std::cout << std::endl;
        }

        // 推进 ARRAY 迭代器到下一个 DICT_ENTRY（或到 DBUS_TYPE_INVALID）
        dbus_message_iter_next(&arr);
    }

    dbus_message_unref(reply);
}

int main()
{
    DBusError err;
    dbus_error_init(&err);

    // 连接到 Session Bus
    DBusConnection *conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        std::cerr << "[client] Connection error: "
                  << err.message << std::endl;
        dbus_error_free(&err);
        return EXIT_FAILURE;
    }

    // 1. 先读取全部属性（验证初始值）
    call_get_all(conn);

    // 2. 单独读取两个属性
    call_get(conn, "PowerStatus");
    call_get(conn, "Volume");

    // 3. 修改 PowerStatus 属性
    call_set(conn, "PowerStatus", false);

    // 4. 再次读取验证修改结果
    call_get(conn, "PowerStatus");

    // 5. 再次读取全部属性（对比前后差异）
    call_get_all(conn);

    dbus_connection_unref(conn);
    return EXIT_SUCCESS;
}
