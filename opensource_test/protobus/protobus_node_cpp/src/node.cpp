#include <iostream>
#include "protobus.hpp"
#include <libgen.h>
#include <unistd.h>
#include "google/protobuf/util/time_util.h"
#include <sstream>
#include <iomanip>
#include <random>
#include <string>
#include <thread>
#include <sys/time.h>
using namespace std;
using namespace std::chrono_literals;
using google::protobuf::Timestamp;
using google::protobuf::util::TimeUtil;
protobus_handle_t *handle = nullptr;
std::vector<std::string> topics;
std::string timestamp_to_string(uint64_t timestamp)
{
    std::time_t time = timestamp / 1000000; // 转换到秒
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
void protobus_callback(const MSG::WrapperMessage &msg)
{
    uint64_t time = msg.timestamp().seconds() * 1000000 + msg.timestamp().nanos();
    switch (msg.message_type_case())
    {
    case MSG::WrapperMessage::kPeople:
    {
        const MSG::msg_people &people_msg = msg.people();
        std::cout << "[" << timestamp_to_string(time) << "] "
                  << "Received message on topic " << msg.topic()
                  << ": Name = " << people_msg.name()
                  << ", Age = " << people_msg.age() << std::endl;
    }
    break;
    case MSG::WrapperMessage::kAddress:
    {
        const MSG::msg_address &addr = msg.address();
        std::cout << "[" << timestamp_to_string(time) << "] "
                  << "Received message on topic " << msg.topic()
                  << ": City = " << addr.city()
                  << ", Street = " << addr.street() << std::endl;
    }
    break;
    default:
    {
        std::cout << "unknow topic " << msg.topic();
    }
    break;
    }
}
int main(int argc, char **argv)
{
    int nodeType = 0;
    if (strcmp(argv[1], "sub") == 0)
    {
        nodeType = 0;
    }
    else
    {
        nodeType = 1;
    }

    std::cout << argv[1] << " node ...." << std::endl;
    if (nodeType == 0)
    {
        for (int i = 2; i < argc; i++)
        {
            std::string str = "argv[" + std::to_string(i) + "] = ";
            std::cout << str << argv[i] << std::endl;
            topics.push_back(argv[i]);
        }

        handle = protobus_init(basename(argv[0]), topics, protobus_callback);
    }
    else
    {
        handle = protobus_init(basename(argv[0]));
    }
    while (1)
    {
        sleep(1);
        if (nodeType == 1)
        {
            std::random_device rd;
            std::mt19937 mt(rd());
            std::uniform_int_distribution<int> dist(0, 100); // 0到100的随机数
            MSG::WrapperMessage wrapper_msg;
            MSG::msg_people *people_msg;
            MSG::msg_address *addr_msg;
            const int randomNum = dist(mt);
            if (randomNum % 2 == 0)
            {
                std::cout << "send address" << std::endl;
                wrapper_msg.set_topic("address");
                addr_msg = wrapper_msg.mutable_address();
                addr_msg->set_city("abc");
                addr_msg->set_street("567");
            }
            else
            {
                std::cout << "send people" << std::endl;
                people_msg = new MSG::msg_people();
                people_msg->set_name("arno");
                people_msg->set_age(20);
                wrapper_msg.set_topic("people");
                wrapper_msg.set_allocated_people(people_msg);
            }
            Timestamp timestamp;
            timestamp.set_seconds(time(NULL));
            timestamp.set_nanos(0);
            *wrapper_msg.mutable_timestamp() = timestamp;
            protobus_send(handle, wrapper_msg);
        }
    }
    protobus_cleanup(handle);

    return 0;
}