#include "iostream"
#include "test_json.pb.h"
#include "google/protobuf/util/json_util.h"

using namespace google::protobuf::util;
void SerializeToJson(const JSON_TEST::json &content)
{
    std::string jsonStr;
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    // 强制输出所有基本类型字段,如果不开启默认值不会输出到json中
    options.always_print_primitive_fields = true; 
    options.preserve_proto_field_names = true;
    MessageToJsonString(content, &jsonStr, options);

    std::cout << "Serialized JSON:\n" << jsonStr << std::endl;
}
int main()
{
    
    JSON_TEST::json content;
    content.set_int_(1);
    content.set_double_(3.1415926);
    content.set_string("This is a string");
    JSON_TEST::json_name_age *p = content.mutable_object();
    p->set_name("arno");
    p->set_age(25);

    content.add_intarray(10);
    content.add_intarray(20);
    content.add_intarray(30);

    content.add_doublearray(0.12345);
    content.add_doublearray(1.12345);
    content.add_doublearray(2.12345);
    content.add_doublearray(3.12345);

    content.add_stringarray("one");
    content.add_stringarray("two");
    content.add_stringarray("three");

    // 添加 People 数据
    JSON_TEST::Person* person = content.add_people();
    person->set_name("arno");
    person->set_age(0);
    person->set_sex(true);

    person = content.add_people();
    person->set_name("arno");
    person->set_age(10);
    person->set_sex(false);

    person = content.add_people();
    person->set_name("arno");
    person->set_age(20);
    person->set_sex(true);

    content.mutable_map_str_str()->insert({"aaa", "111"});
    content.mutable_map_str_str()->insert({"bbb", "222"});
    content.mutable_map_str_str()->insert({"ccc", "333"});
    content.mutable_map_str_str()->insert({"ddd", "444"});

    content.mutable_map_int_str()->insert({111,"aaa"});
    content.mutable_map_int_str()->insert({222,"bbb"});
    content.mutable_map_int_str()->insert({333,"ccc"});
    content.mutable_map_int_str()->insert({444,"ddd"});

    content.mutable_map_str_int()->insert({"aaa", 111});
    content.mutable_map_str_int()->insert({"bbb", 222});
    content.mutable_map_str_int()->insert({"ccc", 333});
    content.mutable_map_str_int()->insert({"ddd", 444});


    SerializeToJson(content);
    return 0;
}
