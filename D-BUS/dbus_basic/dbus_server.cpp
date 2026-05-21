#include <dbus/dbus.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

// =============================================================================
// D-Bus 身份标识（三段定位体系）
// =============================================================================
// Service Name  — 全局唯一服务名，类似域名，客户端通过它找到本进程
// Object Path   — 对象路径，服务内可能有多个对象，每个对象有独立路径
// Interface     — 接口名，一个对象可以实现多个接口，每个接口包含一组方法/信号/属性
constexpr const char *SERVICE_NAME = "com.example.DBusTutorial";
constexpr const char *OBJECT_PATH  = "/com/example/Tutorial";
constexpr const char *INTERFACE    = "com.example.DBusTutorial";

// =============================================================================
// D-Bus 消息处理器（vtable 回调）
// =============================================================================
// 当客户端向本对象发送消息时，dbus-daemon 会将消息分发到这个函数。
// 参数：
//   conn  — 当前 D-Bus 连接
//   msg   — 收到的消息对象（需手动判断消息类型和方法名）
//   data  — 注册 vtable 时传入的 user_data（本示例未使用）
// 返回：
//   DBUS_HANDLER_RESULT_HANDLED        — 已处理，不再传递给其他 handler
//   DBUS_HANDLER_RESULT_NOT_YET_HANDLED — 未处理，允许其他 handler 处理
static DBusHandlerResult handle_message(DBusConnection *conn,
                                        DBusMessage *msg,
                                        void * /*user_data*/)
{
    // 只处理 com.example.DBusTutorial.Add 方法调用，其他消息忽略
    if (!dbus_message_is_method_call(msg, INTERFACE, "Add")) {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    DBusError err;
    dbus_error_init(&err);

    // 从消息中按顺序提取参数：两个 int32
    // DBUS_TYPE_INT32 指定参数类型，&a/&b 分别接收值
    // DBUS_TYPE_INVALID 标记参数列表结束
    dbus_int32_t a = 0, b = 0;
    if (!dbus_message_get_args(msg, &err,
                               DBUS_TYPE_INT32, &a,
                               DBUS_TYPE_INT32, &b,
                               DBUS_TYPE_INVALID)) {
        std::cerr << "[server] Failed to parse Add args: "
                  << err.message << std::endl;
        dbus_error_free(&err);
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    // 业务逻辑：执行加法
    dbus_int32_t result = a + b;
    std::cout << "[server] Add(" << a << ", " << b << ") = "
              << result << std::endl;

    // 构造方法返回消息（METHOD_RETURN）
    // dbus_message_new_method_return 以收到的请求消息为模板创建回复
    DBusMessage *reply = dbus_message_new_method_return(msg);
    if (!reply) {
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    // 将返回值 int32 追加到回复消息中
    dbus_message_append_args(reply,
                             DBUS_TYPE_INT32, &result,
                             DBUS_TYPE_INVALID);

    // 发送回复（异步发送，不阻塞）
    // 第三个参数 nullptr 表示不关心序列号
    dbus_connection_send(conn, reply, nullptr);

    // 释放 reply 消息引用计数
    // D-Bus 消息使用引用计数管理内存，每次 new 或 ref 都需要对应的 unref
    dbus_message_unref(reply);

    return DBUS_HANDLER_RESULT_HANDLED;
}

// =============================================================================
// D-Bus 对象虚拟函数表（ObjectPath VTable）
// =============================================================================
// 注册对象路径时需要提供此结构体，通知 dbus-daemon 如何响应消息。
// message_function — 收到消息时的回调
// unregister_function — 取消注册时的回调（本示例未使用）
// 其余四个 pad 字段是 libdbus 预留的内部字段，必须初始化为 nullptr
static DBusObjectPathVTable vtable = {
    .unregister_function = nullptr,
    .message_function    = handle_message,
    nullptr,  // dbus_internal_pad1
    nullptr,  // dbus_internal_pad2
    nullptr,  // dbus_internal_pad3
    nullptr,  // dbus_internal_pad4
};

int main()
{
    DBusError err;
    dbus_error_init(&err);

    // 1. 连接到 Session Bus
    // DBUS_BUS_SESSION 表示连接用户会话总线（非系统总线）
    // 连接地址从环境变量 DBUS_SESSION_BUS_ADDRESS 获取
    DBusConnection *conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        std::cerr << "[server] Connection error: "
                  << err.message << std::endl;
        dbus_error_free(&err);
        return EXIT_FAILURE;
    }
    if (!conn) {
        std::cerr << "[server] dbus_bus_get returned null" << std::endl;
        return EXIT_FAILURE;
    }

    // 2. 请求 Service Name（服务名注册）
    // DBUS_NAME_FLAG_REPLACE_EXISTING — 如果该名称已被占用，尝试替换原所有者
    // 返回值 DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER 表示成功成为主所有者
    int ret = dbus_bus_request_name(conn, SERVICE_NAME,
                                    DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if (dbus_error_is_set(&err)) {
        std::cerr << "[server] Name error: "
                  << err.message << std::endl;
        dbus_error_free(&err);
        return EXIT_FAILURE;
    }
    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        std::cerr << "[server] Not primary owner (ret=" << ret << ")"
                  << std::endl;
        return EXIT_FAILURE;
    }

    // 3. 注册对象路径 + 绑定消息处理器
    // 此后，发送到 SERVICE_NAME + OBJECT_PATH 的消息将触发 handle_message
    if (!dbus_connection_register_object_path(conn, OBJECT_PATH,
                                               &vtable, nullptr)) {
        std::cerr << "[server] Failed to register object path"
                  << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "[server] Listening on " << SERVICE_NAME
              << " at " << OBJECT_PATH << std::endl;

    // 4. 进入事件循环
    // dbus_connection_read_write_dispatch 执行三个动作：
    //   read  — 从 socket 读取待处理数据
    //   write — 将缓冲区的消息写入 socket
    //   dispatch — 将收到的消息分发给对应的 handler
    // 参数 -1 表示无限等待，直到有数据可读
    while (dbus_connection_read_write_dispatch(conn, -1)) {
    }

    // 5. 释放连接（减少引用计数，当计数归零时关闭连接）
    dbus_connection_unref(conn);
    return EXIT_SUCCESS;
}
