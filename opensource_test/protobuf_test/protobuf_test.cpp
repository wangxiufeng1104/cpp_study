#include "addressbook.pb.h"
#include <google/protobuf/util/json_util.h>
#include <fstream>
#include <iostream>

int main() {
    // 创建一个 AddressBook 对象
    AddressBook address_book;

    // 向 AddressBook 中添加 Person
    Person* person = address_book.add_people();
    person->set_name("John Doe");
    person->set_id(1234);
    person->set_email("johndoe@example.com");

    // 序列化 AddressBook 到 JSON 字符串
    std::string json_output;
    google::protobuf::util::MessageToJsonString(address_book, &json_output);

    // 输出 JSON 字符串
    std::cout << "JSON Output:\n" << json_output << std::endl;

    // 保存 JSON 到文件
    std::ofstream json_file("addressbook.json");
    json_file << json_output;
    json_file.close();

    // 从 JSON 字符串反序列化 AddressBook
    AddressBook new_address_book;
    google::protobuf::util::JsonStringToMessage(json_output, &new_address_book);

    // 输出反序列化后的 AddressBook 内容
    for (const auto& person : new_address_book.people()) {
        std::cout << "Name: " << person.name() << std::endl;
        std::cout << "ID: " << person.id() << std::endl;
        std::cout << "Email: " << person.email() << std::endl;
    }

    return 0;
}
