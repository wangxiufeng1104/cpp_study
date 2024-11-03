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
//#define INDEPENDENT_CALLBACK
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
#ifdef INDEPENDENT_CALLBACK
void addr_callback(const MSG::WrapperMessage &msg)
{
    static uint64_t address_recv_count = 0;
    static uint64_t people_recv_count = 0;

    printf("_____________________addr_callback_________________________\n");

    uint64_t time = msg.timestamp().seconds() * 1000000 + msg.timestamp().nanos();
    switch (msg.message_type_case())
    {
    case MSG::WrapperMessage::kAddress:
    {
        const MSG::msg_address &addr = msg.address();
        std::cout << "[" << timestamp_to_string(time) << "] "
                  << "Received message on topic " << msg.topic()
                  << ": City = " << addr.city()
                  << ", Street = " << addr.street()
                  << ", count = " << addr.count()
                  << ", recv count = " << ++address_recv_count << std::endl;
    }
    break;
    default:
    {
        std::cout << "unknow topic " << msg.topic();
    }
    break;
    }
}
void people_callback(const MSG::WrapperMessage &msg)
{
    static uint64_t address_recv_count = 0;
    static uint64_t people_recv_count = 0;
    printf("_____________________people_callback_________________________\n");

    uint64_t time = msg.timestamp().seconds() * 1000000 + msg.timestamp().nanos();
    switch (msg.message_type_case())
    {
    case MSG::WrapperMessage::kPeople:
    {
        const MSG::msg_people &people_msg = msg.people();
        std::cout << "[" << timestamp_to_string(time) << "] "
                  << "Received message on topic " << msg.topic()
                  << ": Name = " << people_msg.name()
                  << ", Age = " << people_msg.age()
                  << ", count = " << people_msg.count()
                  << ", recv count = " << ++people_recv_count << std::endl;
    }
    break;
    case MSG::WrapperMessage::kAddress:
    {
        const MSG::msg_address &addr = msg.address();
        std::cout << "[" << timestamp_to_string(time) << "] "
                  << "Received message on topic " << msg.topic()
                  << ": City = " << addr.city()
                  << ", Street = " << addr.street()
                  << ", count = " << addr.count()
                  << ", recv count = " << ++address_recv_count << std::endl;
    }
    break;
    default:
    {
        std::cout << "unknow topic " << msg.topic();
    }
    break;
    }
}
#else
void protobus_callback(const MSG::WrapperMessage &msg)
{
    static uint64_t address_recv_count = 0;
    static uint64_t people_recv_count = 0;

    uint64_t time = msg.timestamp().seconds() * 1000000 + msg.timestamp().nanos();
    switch (msg.message_type_case())
    {
    case MSG::WrapperMessage::kPeople:
    {
        const MSG::msg_people &people_msg = msg.people();
        std::cout << "[" << timestamp_to_string(time) << "] "
                  << "Received message on topic " << msg.topic()
                  << ": Name = " << people_msg.name()
                  << ", Age = " << people_msg.age()
                  << ", count = " << people_msg.count()
                  << ", recv count = " << ++people_recv_count << std::endl;
    }
    break;
    case MSG::WrapperMessage::kAddress:
    {
        const MSG::msg_address &addr = msg.address();
        std::cout << "[" << timestamp_to_string(time) << "] "
                  << "Received message on topic " << msg.topic()
                  << ": City = " << addr.city()
                  << ", Street = " << addr.street()
                  << ", count = " << addr.count()
                  << ", recv count = " << ++address_recv_count << std::endl;
    }
    break;
    default:
    {
        std::cout << "unknow topic " << msg.topic();
    }
    break;
    }
}
#endif
MSG::WrapperMessage wrapper_msg;
MSG::msg_people people_msg;
MSG::msg_address addr_msg;
#define SEND_COUNT 2000000
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
#ifdef INDEPENDENT_CALLBACK
        handle = protobus_init(basename(argv[0]));

        protobus_add_subscriber(handle, "people", people_callback);
        protobus_add_subscriber(handle, "address", addr_callback);
#else
        handle = protobus_init(basename(argv[0]), topics, protobus_callback);
#endif
        
    }
    else
    {
        handle = protobus_init(basename(argv[0]));
    }
    sleep(1);
    while (1)
    {

        if (nodeType == 1)
        {
            for (int i = 0; i < SEND_COUNT; i++)
            {
                if (i % 2 == 0)
                {
                    addr_msg.set_city("abc");
                    addr_msg.set_street("567");
                    uint64_t count = addr_msg.count();
                    // printf("address count %lu\n", count);
                    addr_msg.set_count(count + 1);

                    wrapper_msg.set_topic("address");
                    *wrapper_msg.mutable_address() = addr_msg;
                }
                else
                {
                    people_msg.set_name("arno");
                    people_msg.set_age(20);
                    uint64_t count = people_msg.count();
                    // printf("                          people count %lu\n", count);
                    people_msg.set_count(count + 1);

                    wrapper_msg.set_topic("people");
                    *wrapper_msg.mutable_people() = people_msg;
                }
                Timestamp timestamp;
                timestamp.set_seconds(time(NULL));
                timestamp.set_nanos(0);
                *wrapper_msg.mutable_timestamp() = timestamp;
                protobus_send(handle, wrapper_msg);
            }
            //break;
        }
        else
        {
            sleep(1);
        }
    }
    // 防止主线程退出导致子线程崩溃
    while (1)
    {
        sleep(1);
    }
    protobus_cleanup(handle);

    return 0;
}