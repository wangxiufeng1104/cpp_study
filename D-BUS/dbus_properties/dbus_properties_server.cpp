#include <dbus/dbus.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <unistd.h>

// =============================================================================
// D-Bus 标准 Properties 接口 — 服务端实现
// =============================================================================
// org.freedesktop.DBus.Properties 是 D-Bus 规范定义的标准接口。
// 所有 D-Bus 对象都应实现此接口，以支持属性读写。
//
// 三个方法：
//   Get(STRING iface, STRING prop) → VARIANT value      读取单个属性
//   Set(STRING iface, STRING prop, VARIANT value)        写入单个属性
//   GetAll(STRING iface) → DICT<STRING,VARIANT>         读取全部属性
//
// 本示例演示的核心知识点：
//   - VARIANT 类型的序列化（open_container / close_container）
//   - DICT_ENTRY 的构造（All 返回值是 ARRAY of DICT_ENTRY）
//   - DBusMessageIter 迭代器遍历（用于解析复合类型参数）
//   - 方法名分发（通过 dbus_message_get_member 获取方法名）
// =============================================================================

constexpr const char *SERVICE_NAME = "com.example.DBusTutorial";
constexpr const char *OBJECT_PATH  = "/com/example/Tutorial";
constexpr const char *USER_IFACE   = "com.example.DBusTutorial.Properties";

// =============================================================================
// 模拟的属性存储（真实场景可能是硬件状态/配置文件的映射）
// =============================================================================
static struct {
    bool power_status = true;    // 布尔属性示例
    dbus_int32_t volume = 50;   // 整数属性示例
} g_properties;

// =============================================================================
// VARIANT 序列化辅助函数
// =============================================================================
// D-Bus VARIANT 是一种"带运行时类型信息的容器"。
// 序列化一个 VARIANT 的步骤：
//   1. open_container  — 打开 VARIANT 容器，声明内部类型签名
//   2. append_basic    — 向子迭代器写入实际值
//   3. close_container — 关闭容器
// 反序列化时对应：recurse → get_arg_type → get_basic

// 将 bool 值包装为 VARIANT 并追加到迭代器
static void append_variant_bool(DBusMessageIter *iter, bool value)
{
    DBusMessageIter sub;
    // 打开 VARIANT 容器，DBUS_TYPE_BOOLEAN_AS_STRING = "b"
    dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT,
                                     DBUS_TYPE_BOOLEAN_AS_STRING, &sub);
    // 向子迭代器写入布尔值（注意：libdbus 的布尔类型是 dbus_bool_t，即 uint32）
    dbus_bool_t dbus_val = value ? TRUE : FALSE;
    dbus_message_iter_append_basic(&sub, DBUS_TYPE_BOOLEAN, &dbus_val);
    // 关闭容器，标记 VARIANT 结束
    dbus_message_iter_close_container(iter, &sub);
}

// 将 int32 值包装为 VARIANT 并追加到迭代器
static void append_variant_int32(DBusMessageIter *iter, dbus_int32_t value)
{
    DBusMessageIter sub;
    dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT,
                                     DBUS_TYPE_INT32_AS_STRING, &sub);
    dbus_message_iter_append_basic(&sub, DBUS_TYPE_INT32, &value);
    dbus_message_iter_close_container(iter, &sub);
}

// =============================================================================
// handle_get — 实现 Properties.Get(STRING iface, STRING prop) → VARIANT
// =============================================================================
static DBusMessage *handle_get(DBusMessage *msg)
{
    DBusError err;
    dbus_error_init(&err);

    // 提取两个 STRING 参数：接口名、属性名
    const char *iface  = nullptr;
    const char *prop   = nullptr;
    if (!dbus_message_get_args(msg, &err,
                               DBUS_TYPE_STRING, &iface,
                               DBUS_TYPE_STRING, &prop,
                               DBUS_TYPE_INVALID)) {
        // 参数解析失败 → 返回 ERROR 消息（非 METHOD_RETURN）
        DBusMessage *err_reply = dbus_message_new_error(msg, err.name, err.message);
        dbus_error_free(&err);
        return err_reply;
    }

    // 构造 METHOD_RETURN，用迭代器追加 VARIANT 返回值
    DBusMessage *reply = dbus_message_new_method_return(msg);
    DBusMessageIter iter;
    dbus_message_iter_init_append(reply, &iter);

    std::string prop_name(prop ? prop : "");

    // 根据属性名返回对应的 VARIANT 值
    if (prop_name == "PowerStatus") {
        append_variant_bool(&iter, g_properties.power_status);
    } else if (prop_name == "Volume") {
        append_variant_int32(&iter, g_properties.volume);
    } else {
        // 属性不存在 → 返回标准错误
        dbus_message_unref(reply);
        return dbus_message_new_error(msg,
            "org.freedesktop.DBus.Error.UnknownProperty",
            "Property not found");
    }

    return reply;
}

// =============================================================================
// handle_set — 实现 Properties.Set(STRING iface, STRING prop, VARIANT value)
// =============================================================================
// Set 的参数包含 VARIANT 类型，不能用 dbus_message_get_args 直接解析
// （dbus_message_get_args 只能处理基本类型）。
// 必须使用 DBusMessageIter 逐层遍历。
static DBusMessage *handle_set(DBusMessage *msg)
{
    DBusMessageIter iter;
    dbus_message_iter_init(msg, &iter);

    // --- arg 0: STRING interface_name ---
    // 先用 get_arg_type 检查类型，再用 get_basic 读取值
    if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING) {
        return dbus_message_new_error(msg, DBUS_ERROR_INVALID_ARGS,
                                      "Expected STRING");
    }
    const char *iface = nullptr;
    dbus_message_iter_get_basic(&iter, &iface);

    // 验证接口名是否为当前服务支持的接口
    if (!iface || std::string(iface) != USER_IFACE) {
        return dbus_message_new_error(msg,
            "org.freedesktop.DBus.Error.UnknownInterface",
            "Interface not supported");
    }

    // --- arg 1: STRING property_name ---
    // dbus_message_iter_next 将迭代器移动到下一个参数
    if (!dbus_message_iter_next(&iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING) {
        return dbus_message_new_error(msg, DBUS_ERROR_INVALID_ARGS,
                                      "Expected STRING");
    }
    const char *prop = nullptr;
    dbus_message_iter_get_basic(&iter, &prop);

    // --- arg 2: VARIANT value ---
    // VARIANT 是容器类型，需要用 recurse 进入子迭代器
    if (!dbus_message_iter_next(&iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT) {
        return dbus_message_new_error(msg, DBUS_ERROR_INVALID_ARGS,
                                      "Expected VARIANT");
    }

    // dbus_message_iter_recurse 进入 VARIANT 内部
    // 返回的子迭代器指向容器内的实际数据
    DBusMessageIter sub;
    dbus_message_iter_recurse(&iter, &sub);
    int val_type = dbus_message_iter_get_arg_type(&sub);  // 获取内部实际类型

    std::string prop_name(prop ? prop : "");

    // 根据属性名和运行时类型更新内部状态
    if (prop_name == "PowerStatus") {
        // 类型检查：属性声明为 bool，VARIANT 内部必须是 BOOLEAN
        if (val_type != DBUS_TYPE_BOOLEAN) {
            return dbus_message_new_error(msg, DBUS_ERROR_INVALID_ARGS,
                                          "PowerStatus requires boolean");
        }
        dbus_bool_t v = FALSE;
        dbus_message_iter_get_basic(&sub, &v);
        g_properties.power_status = (v == TRUE);
        std::cout << "[server] PowerStatus set to "
                  << (g_properties.power_status ? "true" : "false")
                  << std::endl;
    } else if (prop_name == "Volume") {
        if (val_type != DBUS_TYPE_INT32) {
            return dbus_message_new_error(msg, DBUS_ERROR_INVALID_ARGS,
                                          "Volume requires int32");
        }
        dbus_int32_t v = 0;
        dbus_message_iter_get_basic(&sub, &v);
        g_properties.volume = v;
        std::cout << "[server] Volume set to " << v << std::endl;
    } else {
        return dbus_message_new_error(msg,
            "org.freedesktop.DBus.Error.UnknownProperty",
            "Property not found");
    }

    // Set 成功 → 返回空 METHOD_RETURN（无返回值）
    return dbus_message_new_method_return(msg);
}

// =============================================================================
// handle_get_all — 实现 Properties.GetAll(STRING iface) → ARRAY<DICT<STRING,VARIANT>>
// =============================================================================
// GetAll 的返回值是最复杂的 D-Bus 复合类型：ARRAY of DICT_ENTRY
// 构造步骤：
//   1. open_container  ARRAY，签名使用预定义宏拼接
//   2. 对每个属性，open_container DICT_ENTRY
//   3.   append_basic STRING (key)
//   4.   append_variant (value)
//   5.  close_container DICT_ENTRY
//   6. close_container ARRAY
static DBusMessage *handle_get_all(DBusMessage *msg)
{
    DBusError err;
    dbus_error_init(&err);

    // 提取参数：STRING interface_name
    const char *iface = nullptr;
    if (!dbus_message_get_args(msg, &err,
                               DBUS_TYPE_STRING, &iface,
                               DBUS_TYPE_INVALID)) {
        DBusMessage *err_reply = dbus_message_new_error(msg, err.name, err.message);
        dbus_error_free(&err);
        return err_reply;
    }

    if (!iface || std::string(iface) != USER_IFACE) {
        return dbus_message_new_error(msg,
            "org.freedesktop.DBus.Error.UnknownInterface",
            "Interface not supported");
    }

    DBusMessage *reply = dbus_message_new_method_return(msg);
    DBusMessageIter iter, arr, entry;
    dbus_message_iter_init_append(reply, &iter);

    // 打开 ARRAY 容器，签名 = "a{sv}"（即 ARRAY of DICT_ENTRY{STRING, VARIANT}）
    // 签名由三个宏拼接而成：
    //   DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING = "{"
    //   DBUS_TYPE_STRING_AS_STRING           = "s"
    //   DBUS_TYPE_VARIANT_AS_STRING          = "v"
    //   DBUS_DICT_ENTRY_END_CHAR_AS_STRING   = "}"
    // 整体 = "{sv}"，加上外层的 "a" → "a{sv}"
    dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
        DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING  // "{"
        DBUS_TYPE_STRING_AS_STRING            // "s"
        DBUS_TYPE_VARIANT_AS_STRING           // "v"
        DBUS_DICT_ENTRY_END_CHAR_AS_STRING,   // "}"
        &arr);

    // --- 属性 1: PowerStatus (bool) ---
    // 每个 DICT_ENTRY 是一个键值对：key(STRING) + value(VARIANT)
    dbus_message_iter_open_container(&arr, DBUS_TYPE_DICT_ENTRY, nullptr, &entry);
    const char *key1 = "PowerStatus";
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key1);   // key
    append_variant_bool(&entry, g_properties.power_status);             // value
    dbus_message_iter_close_container(&arr, &entry);                    // 关闭 DICT_ENTRY

    // --- 属性 2: Volume (int32) ---
    dbus_message_iter_open_container(&arr, DBUS_TYPE_DICT_ENTRY, nullptr, &entry);
    const char *key2 = "Volume";
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key2);   // key
    append_variant_int32(&entry, g_properties.volume);                  // value
    dbus_message_iter_close_container(&arr, &entry);

    // 关闭 ARRAY 容器
    dbus_message_iter_close_container(&iter, &arr);

    return reply;
}

// =============================================================================
// 消息分发器 — 根据方法名路由到对应的 handler
// =============================================================================
// vtable handler 会收到发往此 Object Path 的所有消息，
// 包括 Introspectable 请求、内部 D-Bus 消息、甚至信号等。
// 必须做三层过滤：消息类型 → 接口名 → 方法名。
static DBusHandlerResult handle_message(DBusConnection *conn,
                                        DBusMessage *msg,
                                        void * /*user_data*/)
{
    // 第 1 层过滤：只处理方法调用（排除信号、错误回复、方法返回等）
    if (dbus_message_get_type(msg) != DBUS_MESSAGE_TYPE_METHOD_CALL) {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    // 第 2 层过滤：只处理 org.freedesktop.DBus.Properties 接口
    const char *iface_name = dbus_message_get_interface(msg);
    if (!iface_name ||
        std::strcmp(iface_name, "org.freedesktop.DBus.Properties") != 0) {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    // 第 3 层：获取方法名并分发
    const char *member = dbus_message_get_member(msg);
    if (!member) {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    DBusMessage *reply = nullptr;
    std::string m(member);

    if (m == "Get") {
        reply = handle_get(msg);
    } else if (m == "Set") {
        reply = handle_set(msg);
    } else if (m == "GetAll") {
        reply = handle_get_all(msg);
    } else {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    if (reply) {
        dbus_connection_send(conn, reply, nullptr);
        dbus_message_unref(reply);
    }

    return DBUS_HANDLER_RESULT_HANDLED;
}

// =============================================================================
// D-Bus 对象虚拟函数表
// =============================================================================
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

    // 2. 请求 Service Name（与 dbus_basic server 共享同一个服务名）
    int ret = dbus_bus_request_name(conn, SERVICE_NAME,
                                    DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if (dbus_error_is_set(&err)) {
        std::cerr << "[server] Name error: "
                  << err.message << std::endl;
        dbus_error_free(&err);
        return EXIT_FAILURE;
    }

    // 3. 注册对象路径 + 绑定消息处理器
    if (!dbus_connection_register_object_path(conn, OBJECT_PATH,
                                               &vtable, nullptr)) {
        std::cerr << "[server] Failed to register object path"
                  << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "[server] Properties service listening on "
              << SERVICE_NAME << " at " << OBJECT_PATH << std::endl;
    std::cout << "[server] Properties: PowerStatus="
              << (g_properties.power_status ? "true" : "false")
              << ", Volume=" << g_properties.volume << std::endl;

    // 4. 进入事件循环
    while (dbus_connection_read_write_dispatch(conn, -1)) {
    }

    dbus_connection_unref(conn);
    return EXIT_SUCCESS;
}
