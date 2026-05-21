/**
 * @file requester.cpp
 * @brief 请求-应答模式请求者示例
 * 
 * 演示如何实现RPC风格的通信：
 * - 发送请求
 * - 接收应答
 */

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include "Common.hpp"
#include "RequestReplyPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;
using namespace fastdds_test;

class Requester
{
public:
    Requester() : participant_(nullptr), publisher_(nullptr), subscriber_(nullptr),
                  request_topic_(nullptr), reply_topic_(nullptr), 
                  writer_(nullptr), reader_(nullptr),
                  request_type_(new RequestReply::AddRequestPubSubType()),
                  reply_type_(new RequestReply::AddReplyPubSubType()) {}

    ~Requester()
    {
        if (writer_) publisher_->delete_datawriter(writer_);
        if (reader_) subscriber_->delete_datareader(reader_);
        if (publisher_) participant_->delete_publisher(publisher_);
        if (subscriber_) participant_->delete_subscriber(subscriber_);
        if (request_topic_) participant_->delete_topic(request_topic_);
        if (reply_topic_) participant_->delete_topic(reply_topic_);
        if (participant_) DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    bool init()
    {
        DomainParticipantQos pqos;
        pqos.name("RequestReply_Requester");
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
        if (!participant_) return false;

        request_type_.register_type(participant_);
        reply_type_.register_type(participant_);

        request_topic_ = participant_->create_topic("AddRequestTopic", request_type_.get_type_name(), TOPIC_QOS_DEFAULT);
        reply_topic_ = participant_->create_topic("AddReplyTopic", reply_type_.get_type_name(), TOPIC_QOS_DEFAULT);

        publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

        writer_ = publisher_->create_datawriter(request_topic_, DATAWRITER_QOS_DEFAULT, nullptr);
        reader_ = subscriber_->create_datareader(reply_topic_, DATAREADER_QOS_DEFAULT, &listener_);

        std::cout << "请求者初始化成功!" << std::endl;
        return true;
    }

    void send_request(int32_t a, int32_t b)
    {
        RequestReply::AddRequest req;
        req.a(a);
        req.b(b);

        if (writer_->write(&req) == ReturnCode_t::RETCODE_OK)
        {
            std::cout << "[" << get_timestamp() << "] 发送请求: " << a << " + " << b << std::endl;
        }
    }

private:
    DomainParticipant* participant_;
    Publisher* publisher_;
    Subscriber* subscriber_;
    Topic* request_topic_;
    Topic* reply_topic_;
    DataWriter* writer_;
    DataReader* reader_;
    TypeSupport request_type_;
    TypeSupport reply_type_;

    class ReplyListener : public DataReaderListener
    {
    public:
        void on_data_available(DataReader* reader) override
        {
            SampleInfo info;
            RequestReply::AddReply reply;
            while (reader->take_next_sample(&reply, &info) == ReturnCode_t::RETCODE_OK)
            {
                if (info.valid_data)
                {
                    std::cout << "[" << get_timestamp() << "] 收到应答: " 
                              << "结果=" << reply.result() 
                              << ", 成功=" << (reply.success() ? "true" : "false")
                              << ", 消息=\"" << reply.message() << "\"" << std::endl;
                }
            }
        }
    } listener_;
};

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "请求-应答模式 请求者示例" << std::endl;
    std::cout << "========================================" << std::endl;

    setup_signal_handler();

    Requester req;
    if (!req.init()) return 1;

    // 等待Replier连接
    std::cout << "等待Replier连接..." << std::endl;
    sleep_ms(2000);

    // 发送多个请求
    int count = 0;
    while (!g_shutdown && count < 10)
    {
        req.send_request(count * 10, count * 5);
        sleep_ms(2000);
        count++;
    }

    std::cout << "请求者退出" << std::endl;
    return 0;
}