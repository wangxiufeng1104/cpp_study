#include <iostream>
#include "contact.pb.h"
#include <fstream>
using namespace std;
const char *file = "123";
const char *title = "dsdasdas";
const char *content = "fdsfsddsf";

int main()
{
    string people_str;
    {
        contacts::PeopleInfo people;
        people.set_age(20);
        people.set_name("arno");
        cout << "people size:" << people.phone_size() << endl;

        contacts::PeopleInfo_Phone *phone = people.add_phone();
        phone->set_tyepe(contacts::PeopleInfo_Phone_PhoneType_MOBILE);
        phone->set_number("2345");
        cout << "people size:" << people.phone_size() << endl;
        people.mutable_remark()->insert({"aaa", "111"});
        people.mutable_remark()->insert({"bbb", "222"});
        people.mutable_remark()->insert({"ccc", "333"});
        people.mutable_remark()->insert({"ddd", "444"});
        people.mutable_remark()->insert({"eee", "555"});
        people.mutable_remark()->insert({"fff", "666"});
        people.mutable_remark()->insert({"ggg", "777"});

        if (!people.SerializeToString(&people_str))
        {
            cerr << "serialize failied" << endl;
            return 0;
        }
        cout << people_str << endl;
        fstream output(file, ios::out | ios::trunc | ios::binary);
        if (!output || !people.SerializePartialToOstream(&output))
        {
            cerr << "write error" << endl;
            exit(3);
        }
    }
    {
        contacts::PeopleInfo people;
        if (!people.ParseFromString(people_str))
        {
            cerr << "parse failied" << endl;
            return 0;
        }
        cout << "反序列化的结果为:" << endl;
        cout << "name: " << people.name() << endl;
        cout << "age: " << people.age() << endl;

        std::for_each(people.remark().begin(), people.remark().end(), [](const std::pair<std::string, std::string> &pair)
                      { std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl; });
    }
    return 0;
}