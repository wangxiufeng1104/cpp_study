/**
 * @file publisher.cpp
 * @brief TCP传输发布者示例
 * 
 * 演示FastDDS的TCP传输配置：
 * - 配置TCP作为传输协议
 * - 设置TCP监听端口
 * - 适用于需要TCP穿越防火墙的场景
 */

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.h>
#include <fastdds/rtps/attributes/ParticipantAttributes.h>

#include "Common.hpp"
#include "HelloWorldPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;
using namespace fastdds_test;

class TCPPublisher
{
public:
    TCPPublisher() : participant_(nullptr), publisher_(nullptr), topic_(nullptr), 
                     writer_(nullptr), type_(new HelloWorldPubSubType()) {}

    ~TCPPublisher()
    {
        if (writer_) publisher_->delete_datawriter(writer_);
        if (publisher_) participant_->delete_publisher(publisher_);
        if (topic_) participant_->delete_topic(topic_);
        if (participant_) DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    bool init(uint16_t port)
    {
        // 配置TCP传输
        DomainParticipantQos part_qos;
        part_qos.name("TCP_Publisher");

        // 禁用默认传输，启用TCP
        part_qos.transport().use_builtin_transports = false;

        // 创建TCP传输描述符
        auto tcp_descriptor = std::make_shared<TCPv4TransportDescriptor>();
        tcp_descriptor->sendBufferSize = 0;  // 默认值
        tcp_descriptor->receiveBufferSize = 0;  // 默认值
        
        // 设置TCP监听端口
        tcp_descriptor->add_listener_port(port);
        
        // 添加到参与者的传输配置
        part_qos.transport().user_transports.push_back(tcp_descriptor);

        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, part_qos);
        if (!participant_) return false;

        type_.register_type(participant_);

        topic_ = participant_->create_topic("TCPTopic", type_.get_type_name(), TOPIC_QOS_DEFAULT);
        if (!topic_) return false;

        publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
        if (!publisher_) return false;

        writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, &listener_);
        if (!writer_) return false;

        std::cout << "TCP发布者初始化成功!" << std::endl;
        std::cout << "TCP监听端口: " << port << std::endl;
        return true;
    }

    bool publish(uint32_t index, const std::string& msg)
    {
        HelloWorld hello;
        hello.index(index);
        hello.message(msg);
        return writer_->write(&hello) == ReturnCode_t::RETCODE_OK;
    }

private:
    DomainParticipant* participant_;
    Publisher* publisher_;
    Topic* topic_;
    DataWriter* writer_;
    TypeSupport type_;

    class PubListener : public DataWriterListener
    {
    public:
        void on_publication_matched(DataWriter*, const PublicationMatchedStatus& info) override
        {
            if (info.current_count_change == 1)
                std::cout << "TCP匹配到订阅者 (匹配数: " << info.current_count << ")" << std::endl;
        }
    } listener_;
};

int main(int argc, char** argv)
{
    std::cout << "========================================" << std::endl;
    std::cout << "TCP传输 发布者示例" << std::endl;
    std::cout << "========================================" << std::endl;

    setup_signal_handler();

    uint16_t port = 5100;
    if (argc > 1)
    {
        port = static_cast<uint16_t>(std::atoi(argv[1]));
    }

    TCPPublisher pub;
    if (!pub.init(port)) return 1;

    std::cout << "\n等待TCP订阅者连接..." << std::endl;
    sleep_ms(2000);

    uint32_t index = 0;
    while (!g_shutdown)
    {
        std::string msg = "TCP Message #" + std::to_string(index);
        if (pub.publish(index, msg))
        {
            std::cout << "[" << get_timestamp() << "] 已发布(TCP): " << msg << std::endl;
        }
        index++;
        sleep_ms(1000);
    }

    return 0;
}