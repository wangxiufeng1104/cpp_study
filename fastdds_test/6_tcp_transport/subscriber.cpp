/**
 * @file subscriber.cpp
 * @brief TCP传输订阅者示例
 */

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>

#include "Common.hpp"
#include "HelloWorldPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;
using namespace fastdds_test;

class TCPSubscriber
{
public:
    TCPSubscriber() : participant_(nullptr), subscriber_(nullptr), topic_(nullptr), 
                      reader_(nullptr), type_(new HelloWorldPubSubType()) {}

    ~TCPSubscriber()
    {
        if (reader_) subscriber_->delete_datareader(reader_);
        if (subscriber_) participant_->delete_subscriber(subscriber_);
        if (topic_) participant_->delete_topic(topic_);
        if (participant_) DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    bool init(const std::string& server_ip, uint16_t port)
    {
        DomainParticipantQos part_qos;
        part_qos.name("TCP_Subscriber");
        part_qos.transport().use_builtin_transports = false;

        auto tcp_descriptor = std::make_shared<TCPv4TransportDescriptor>();
        tcp_descriptor->sendBufferSize = 0;
        tcp_descriptor->receiveBufferSize = 0;

        // 连接到TCP服务器
        tcp_descriptor->add_connection_port(port, server_ip);
        part_qos.transport().user_transports.push_back(tcp_descriptor);

        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, part_qos);
        if (!participant_) return false;

        type_.register_type(participant_);

        topic_ = participant_->create_topic("TCPTopic", type_.get_type_name(), TOPIC_QOS_DEFAULT);
        if (!topic_) return false;

        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
        if (!subscriber_) return false;

        reader_ = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, &listener_);
        if (!reader_) return false;

        std::cout << "TCP订阅者初始化成功!" << std::endl;
        std::cout << "连接到: " << server_ip << ":" << port << std::endl;
        return true;
    }

private:
    DomainParticipant* participant_;
    Subscriber* subscriber_;
    Topic* topic_;
    DataReader* reader_;
    TypeSupport type_;

    class SubListener : public DataReaderListener
    {
    public:
        void on_data_available(DataReader* reader) override
        {
            SampleInfo info;
            HelloWorld hello;
            while (reader->take_next_sample(&hello, &info) == ReturnCode_t::RETCODE_OK)
            {
                if (info.valid_data)
                {
                    std::cout << "[" << get_timestamp() << "] 收到(TCP): index=" 
                              << hello.index() << ", message=\"" << hello.message() << "\"" << std::endl;
                }
            }
        }

        void on_subscription_matched(DataReader*, const SubscriptionMatchedStatus& info) override
        {
            if (info.current_count_change == 1)
                std::cout << "TCP匹配到发布者 (匹配数: " << info.current_count << ")" << std::endl;
        }
    } listener_;
};

int main(int argc, char** argv)
{
    std::cout << "========================================" << std::endl;
    std::cout << "TCP传输 订阅者示例" << std::endl;
    std::cout << "========================================" << std::endl;

    setup_signal_handler();

    std::string server_ip = "127.0.0.1";
    uint16_t port = 5100;

    if (argc > 1) server_ip = argv[1];
    if (argc > 2) port = static_cast<uint16_t>(std::atoi(argv[2]));

    TCPSubscriber sub;
    if (!sub.init(server_ip, port)) return 1;

    std::cout << "等待接收TCP数据，按Ctrl+C退出..." << std::endl;
    while (!g_shutdown) sleep_ms(1000);

    return 0;
}