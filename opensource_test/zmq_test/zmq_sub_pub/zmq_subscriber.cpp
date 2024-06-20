#include <iostream>
#include "zmq/zmq.hpp"
#include "message.pb.h"
#include "google/protobuf/util/time_util.h"
#define TCP_PUB "tcp://127.0.0.1:5556"
using google::protobuf::Timestamp;
using google::protobuf::util::TimeUtil;
std::string timestamp_to_string(uint64_t timestamp)
{
    std::time_t time = timestamp / 1000000; // 转换到秒
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
int main(int argc, char **argv)
{
    std::cout << "start sub node ..." << std::endl;

    zmq::context_t ctx(1);
    zmq::socket_t subscriber(ctx, zmq::socket_type::sub);
    subscriber.connect(TCP_PUB);
    for (int i = 0; i < argc - 1; i++)
    {
        const char *filter = argv[1 + i];
        std::cout << filter << std::endl;
        subscriber.set(zmq::sockopt::subscribe, filter);
    }
    while (1)
    {
        zmq::message_t topic;
        zmq::message_t zmq_msg;
        zmq::recv_result_t result;
        result = subscriber.recv(topic);
        if (result.has_value())
        {
            std::string str(static_cast<char *>(topic.data()), topic.size());
            std::cout << "recv data : " << str << std::endl;
        }
        else
        {
            continue;
        }
        result = subscriber.recv(zmq_msg, zmq::recv_flags::none);
        if (result.has_value())
        {
            // 反序列化消息
            MSG::WrapperMessage wrapper_msg;
            wrapper_msg.ParseFromArray(zmq_msg.data(), zmq_msg.size());
            uint64_t time = wrapper_msg.timestamp().seconds() * 1000000 + wrapper_msg.timestamp().nanos();

            switch (wrapper_msg.message_type_case())
            {
            case MSG::WrapperMessage::kPeople:
            {
                const MSG::msg_people &people_msg = wrapper_msg.people();
                std::cout << "[" << timestamp_to_string(time) << "] "
                          << "Received message on topic " << wrapper_msg.topic()
                          << ": Name = " << people_msg.name()
                          << ", Age = " << people_msg.age() << std::endl;
            }
            break;
            case MSG::WrapperMessage::kAddress:
            {
                const MSG::msg_address &addr = wrapper_msg.address();
                std::cout << "[" << timestamp_to_string(time) << "] "
                          << "Received message on topic " << wrapper_msg.topic()
                          << ": City = " << addr.city()
                          << ", Street = " << addr.street() << std::endl;
            }
            break;
            default:
            {
                std::cout << "unknow topic " << wrapper_msg.topic();
            }
            break;
            }
        }
        else
        {
            std::cerr << "Failed to receive message" << std::endl;
        }
    }
    return 0;
}