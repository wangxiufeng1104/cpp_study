#include <iostream>
#include "jsoncpp/json.h"

int main()
{
    Json::Value root;
    root["name"] = "John";
    root["age"] = 30;
    root["city"] = "New York";

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
    return 0;
}