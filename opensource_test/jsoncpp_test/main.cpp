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
void Serialize()
{
    Json::Value root;
    root["Int"] = 1;
    root["Double"] = 3.1415926;
    root["String"] = "This is a string";

    // 添加嵌套对象
    Json::Value nestedObject;
    nestedObject["name"] = "arno";
    nestedObject["age"] = 25;
    root["Object"] = nestedObject;

    // 添加整数数组
    Json::Value intArray(Json::arrayValue);
    intArray.append(10);
    intArray.append(20);
    intArray.append(30);
    root["IntArray"] = intArray;

    // 添加浮点数数组
    Json::Value doubleArray(Json::arrayValue);
    doubleArray.append(0.0);
    doubleArray.append(1.0);
    doubleArray.append(2.0);
    doubleArray.append(3.0);
    root["DoubleArray"] = doubleArray;

    // 添加字符串数组
    Json::Value stringArray(Json::arrayValue);
    stringArray.append("one");
    stringArray.append("two");
    stringArray.append("three");
    root["StringArray"] = stringArray;

    // 添加混合类型数组
    Json::Value mixedArray(Json::arrayValue);
    mixedArray.append("one");
    mixedArray.append(50);
    mixedArray.append(false);
    mixedArray.append(12.005);
    root["MixedArray"] = mixedArray;

    // 添加对象数组
    Json::Value peopleArray(Json::arrayValue);
    for (int i = 0; i <= 20; i += 10)
    {
        Json::Value person;
        person["name"] = "arno";
        person["age"] = i;
        person["sex"] = (i % 20 == 0); // true for 0 and 20, false for 10
        peopleArray.append(person);
    }
    root["People"] = peopleArray;

    // 将 JSON 对象序列化为字符串
    Json::StreamWriterBuilder writer;
    std::string jsonString = Json::writeString(writer, root);

    // 输出 JSON 字符串
    std::cout << jsonString << std::endl;
}
// 假设这是我们要解析的 JSON 字符串
std::string jsonString = R"({
        "Int": 1,
        "Double": 3.1415926,
        "String": "This is a string",
        "Object": {
            "name": "arno",
            "age": 25
        },
        "IntArray": [10, 20, 30],
        "DoubleArray": [0, 1, 2, 3],
        "StringArray": ["one", "two", "three"],
        "MixedArray": ["one", 50, false, 12.005],
        "People": [
            {"name": "arno", "age": 0, "sex": true},
            {"name": "arno", "age": 10, "sex": false},
            {"name": "arno", "age": 20, "sex": true}
        ]
    })";
void parse()
{
    // 创建一个 JSON 文档对象
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errs;

    // 解析 JSON 字符串
    std::istringstream s(jsonString);
    if (!Json::parseFromStream(reader, s, &root, &errs))
    {
        std::cout << "Error parsing JSON: " << errs << std::endl;
        return;
    }

    // 访问解析后的数据
    // 访问整数
    int intValue = root["Int"].asInt();
    std::cout << "Int: " << intValue << std::endl;

    // 访问浮点数
    double doubleValue = root["Double"].asDouble();
    std::cout << "Double: " << doubleValue << std::endl;

    // 访问字符串
    std::string stringValue = root["String"].asString();
    std::cout << "String: " << stringValue << std::endl;

    // 访问嵌套对象
    std::string nestedName = root["Object"]["name"].asString();
    int nestedAge = root["Object"]["age"].asInt();
    std::cout << "Object - Name: " << nestedName << ", Age: " << nestedAge << std::endl;

    // 访问整数数组
    const Json::Value intArray = root["IntArray"];
    std::cout << "IntArray: ";
    for (const auto &val : intArray)
    {
        std::cout << val.asInt() << " ";
    }
    std::cout << std::endl;

    // 访问浮点数数组
    const Json::Value doubleArray = root["DoubleArray"];
    std::cout << "DoubleArray: ";
    for (const auto &val : doubleArray)
    {
        std::cout << val.asDouble() << " ";
    }
    std::cout << std::endl;

    // 访问字符串数组
    const Json::Value stringArray = root["StringArray"];
    std::cout << "StringArray: ";
    for (const auto &val : stringArray)
    {
        std::cout << val.asString() << " ";
    }
    std::cout << std::endl;

    // 访问混合类型数组
    const Json::Value mixedArray = root["MixedArray"];
    std::cout << "MixedArray: ";
    for (const auto &val : mixedArray)
    {
        if (val.isString())
        {
            std::cout << val.asString() << " ";
        }
        else if (val.isInt())
        {
            std::cout << val.asInt() << " ";
        }
        else if (val.isBool())
        {
            std::cout << (val.asBool() ? "true" : "false") << " ";
        }
        else if (val.isDouble())
        {
            std::cout << val.asDouble() << " ";
        }
    }
    std::cout << std::endl;

    // 访问对象数组
    const Json::Value peopleArray = root["People"];
    for (const auto &person : peopleArray)
    {
        std::string personName = person["name"].asString();
        int personAge = person["age"].asInt();
        bool personSex = person["sex"].asBool();
        std::cout << "Person - Name: " << personName << ", Age: " << personAge << ", Sex: " << (personSex ? "true" : "false") << std::endl;
    }
}
int main()
{
#if 1
    Serialize();
    parse();
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
#elif 0
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