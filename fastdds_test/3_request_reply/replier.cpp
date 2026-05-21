/**
 * @file replier.cpp
 * @brief 请求-应答模式应答者示例
 * 
 * 演示如何实现RPC风格的服务端：
 * - 接收请求
 * - 处理并发送应答
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

class Replier
{
public:
    Replier() : participant_(nullptr), publisher_(nullptr), subscriber_(nullptr),
                request_topic_(nullptr), reply_topic_(nullptr),
                writer_(nullptr), reader_(nullptr),
                request_type_(new RequestReply::AddRequestPubSubType()),
                reply_type_(new RequestReply::AddReplyPubSubType()) {}

    ~Replier()
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
        pqos.name("RequestReply_Replier");
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
        if (!participant_) return false;

        request_type_.register_type(participant_);
        reply_type_.register_type(participant_);

        request_topic_ = participant_->create_topic("AddRequestTopic", request_type_.get_type_name(), TOPIC_QOS_DEFAULT);
        reply_topic_ = participant_->create_topic("AddReplyTopic", reply_type_.get_type_name(), TOPIC_QOS_DEFAULT);

        publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

        writer_ = publisher_->create_datawriter(reply_topic_, DATAWRITER_QOS_DEFAULT, nullptr);
        reader_ = subscriber_->create_datareader(request_topic_, DATAREADER_QOS_DEFAULT, &listener_);

        listener_.set_writer(writer_);

        std::cout << "应答者初始化成功!" << std::endl;
        return true;
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

    class RequestListener : public DataReaderListener
    {
    public:
        void set_writer(DataWriter* writer) { writer_ = writer; }

        void on_data_available(DataReader* reader) override
        {
            SampleInfo info;
            RequestReply::AddRequest req;
            while (reader->take_next_sample(&req, &info) == ReturnCode_t::RETCODE_OK)
            {
                if (info.valid_data && writer_)
                {
                    // 处理请求
                    int32_t result = req.a() + req.b();
                    
                    RequestReply::AddReply reply;
                    reply.result(result);
                    reply.success(true);
                    reply.message("计算成功: " + std::to_string(req.a()) + " + " + std::to_string(req.b()) + " = " + std::to_string(result));

                    writer_->write(&reply);
                    
                    std::cout << "[" << get_timestamp() << "] 处理请求: " 
                              << req.a() << " + " << req.b() << " = " << result << std::endl;
                }
            }
        }
    private:
        DataWriter* writer_ = nullptr;
    } listener_;
};

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "请求-应答模式 应答者示例" << std::endl;
    std::cout << "========================================" << std::endl;

    setup_signal_handler();

    Replier rep;
    if (!rep.init()) return 1;

    std::cout << "等待请求，按Ctrl+C退出..." << std::endl;
    while (!g_shutdown) sleep_ms(1000);

    std::cout << "应答者退出" << std::endl;
    return 0;
}