#include <iostream>
#include "zmq/zmq.hpp"
#include "message.pb.h"
#include <random>
#include <string>
#include <thread>
#define TCP_SUB "tcp://127.0.0.1:5555"
using namespace std::chrono_literals;
int main()
{
    std::cout << "start pub node ..." << std::endl;

    WrapperMessage wrapper_msg;
    msg_people *people_msg;
    msg_address *addr_msg;

    zmq::context_t ctx(1);
    zmq::socket_t publisher(ctx, zmq::socket_type::pub);
    publisher.connect(TCP_SUB);

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(0, 100); // 0到100的随机数
    while (1)
    {
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
            people_msg = new msg_people();
            people_msg->set_name("arno");
            people_msg->set_age(20);
            wrapper_msg.set_topic("people");
            wrapper_msg.set_allocated_people(people_msg);
        }

        // 序列化消息
        std::string serialized_msg;
        wrapper_msg.SerializePartialToString(&serialized_msg);

        // 发送消息
        zmq::message_t topic(wrapper_msg.topic());
        publisher.send(topic, zmq::send_flags::sndmore);
        zmq::message_t zmq_msg(serialized_msg.size());
        memcpy(zmq_msg.data(), serialized_msg.data(), serialized_msg.size());
        publisher.send(zmq_msg, zmq::send_flags::dontwait);

        // 睡眠一秒
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}