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
#include <signal.h>
using namespace std;
using namespace std::chrono_literals;
using google::protobuf::Timestamp;
using google::protobuf::util::TimeUtil;
std::vector<std::string> topics;
int run_status = 1;
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
        std::ostringstream oss;
        oss << "[" << timestamp_to_string(time) << "] "
            << "Received message on topic " << msg.topic()
            << ": Name = " << people_msg.name()
            << ", Age = " << people_msg.age()
            << ", count = " << people_msg.count()
            << ", recv count = " << ++people_recv_count;
        ELELOG_DBG("%s", oss.str().data());
    }
    break;
    case MSG::WrapperMessage::kAddress:
    {
        const MSG::msg_address &addr = msg.address();
        std::ostringstream oss;
        oss << "[" << timestamp_to_string(time) << "] "
            << "Received message on topic " << msg.topic()
            << ": City = " << addr.city()
            << ", Street = " << addr.street()
            << ", count = " << addr.count()
            << ", recv count = " << ++address_recv_count;
        ELELOG_DBG("%s", oss.str().data());
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

void sig_handle(int sig_num)
{
    switch (sig_num)
    {
    case SIGTERM:
    case SIGINT:
    {
        run_status = 0;
    }
    break;
    default:
        break;
    }
}

int main(int argc, char **argv)
{
    signal(SIGTERM, sig_handle);
    signal(SIGINT, sig_handle);
    for (int i = 1; i < argc; i++)
    {
        std::string str = "argv[" + std::to_string(i) + "] = ";
        std::cout << str << argv[i] << std::endl;
        topics.push_back(argv[i]);
    }
    std::shared_ptr bus = protobus::get_instance(basename(argv[0]), topics, protobus_callback);
    while (run_status)
    {
        sleep(1);
    }
    std::cout << "good bye" << std::endl;
    return 0;
}