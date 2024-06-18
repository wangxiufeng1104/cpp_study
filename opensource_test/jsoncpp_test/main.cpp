#include <iostream>
#include "jsoncpp/json.h"


// 全局变量
static Json::Value param_cache;
static pthread_mutex_t param_mutex = PTHREAD_MUTEX_INITIALIZER;
char update_param(const char *name, void *value)
{
    uint8_t ret_val = 0;

    pthread_mutex_lock(&param_mutex);

    // 遍历 param_cache
    for (const auto &key : param_cache.getMemberNames())
    {
        if (key == name) // 找到缓存中的参数
        {
            const Json::Value &param_value = param_cache[key];
            const Json::Value &val = param_value["Value"];

            if (val.isString())
            {
                // printf("C APP: String, Value: %s\n", val.asCString());
                strcpy(static_cast<char *>(value), val.asCString());
            }
            else if (val.isInt())
            {
                // printf("C APP: Integer, Value: %d\n", val.asInt());
                *(static_cast<int *>(value)) = val.asInt();
            }
            else if (val.isBool())
            {
                std::cout << "C APP: Boolean, Value: " << (val.asBool() ? "true" : "false") << std::endl;
                *(static_cast<bool *>(value)) = val.asBool();
            }
            else
            {
                ret_val = -1;
                std::cout << "C APP: Unknown" << std::endl;
            }
            break;
        }
    }

    pthread_mutex_unlock(&param_mutex);

    return ret_val;
}
enum val_type_t
{
    STRING_TYPE,
    INT_TYPE,
    BOOL_TYPE
};
uint8_t report_param_change(const char *name, const void *value, val_type_t val_type)
{
    Json::Value param_object;
    param_object["ParamName"] = name;

    if (val_type == STRING_TYPE)
    {
        param_object["Value"] = static_cast<const char *>(value);
    }
    else if (val_type == INT_TYPE)
    {
        param_object["Value"] = *(static_cast<const int *>(value));
    }
    else if (val_type == BOOL_TYPE)
    {
        param_object["Value"] = *(static_cast<const bool *>(value));
    }

    // 将 param_object 添加到全局 JSON 对象 param_cache 中
    param_cache[name] = param_object;

    // 将 JSON 对象转换为字符串
    Json::StreamWriterBuilder writer;
    std::string json_str = Json::writeString(writer, param_object);
    std::cout << json_str << std::endl;

    // 准备消息
    // struct message msg;
    // msg.mtype = 23;
    // memset(msg.mtext, 0, sizeof(msg.mtext));
    // strncpy(msg.mtext, json_str.c_str(), sizeof(msg.mtext) - 1); // 确保不超过缓冲区大小

    // // 发送消息
    // if (msgsnd(msqid, &msg, sizeof(msg.mtext), 0) == -1) {
    //     perror("msgsnd");
    //     return -1;
    // }

    return 0;
}
int main()
{
#if 0
    Json::Value root;
    root["name"] = "John";
    root["age"] = 30;
    root["city"] = "New York";
    root["isAlive"] = true;
    root["isStudent"] = false;

    // 序列化为字符串
    Json::StreamWriterBuilder writer;
    std::string output = Json::writeString(writer, root);
    std::cout << output << std::endl;

    // 从字符串反序列化
    std::string jsonString = R"({"name":"John","age":30,"city":"New York"})";
    Json::CharReaderBuilder reader;
    Json::Value rootParsed;
    std::string errs;
    std::istringstream s(jsonString);
    if(Json::parseFromStream(reader, s, &rootParsed, &errs))
    {
        std::cout << rootParsed["name"].asString() << std::endl;
        std::cout << rootParsed["age"].asString() << std::endl;
        std::cout << rootParsed["city"].asString() << std::endl;
    }
    else
    {
        std::cerr << "failed to parse JSON" << errs << std::endl;
    }
#elif 0
    // 初始化示例数据
    param_cache["example_string"]["Value"] = "example_value";
    param_cache["example_int"]["Value"] = 42;
    param_cache["example_bool"]["Value"] = true;
    Json::StreamWriterBuilder writer;

    // 测试 update_param 函数
    char string_value[50];
    int int_value;
    bool bool_value;

    update_param("example_string", string_value);
    std::cout << "Updated string: " << string_value << std::endl;

    update_param("example_int", &int_value);
    std::cout << "Updated int: " << int_value << std::endl;

    update_param("example_bool", &bool_value);
    std::cout << "Updated bool: " << (bool_value ? "true" : "false") << std::endl;

    std::string output = Json::writeString(writer, param_cache);
    std::cout << output << std::endl;
#elif 1
    // 示例测试
    const char *param_name = "example_param";
    const char *string_value = "example_value";
    int int_value = 42;
    bool bool_value = true;

    report_param_change(param_name, string_value, STRING_TYPE);
    report_param_change(param_name, &int_value, INT_TYPE);
    report_param_change(param_name, &bool_value, BOOL_TYPE);
#endif
    return 0;
}