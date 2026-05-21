/**
 * @file subscriber.cpp
 * @brief QoS配置订阅者示例
 */

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>

#include "Common.hpp"
#include "HelloWorldPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;
using namespace fastdds_test;

class QoSSubscriber
{
public:
    QoSSubscriber() : participant_(nullptr), subscriber_(nullptr), topic_(nullptr), reader_(nullptr),
                      type_(new HelloWorldPubSubType()) {}

    ~QoSSubscriber()
    {
        if (reader_) subscriber_->delete_datareader(reader_);
        if (subscriber_) participant_->delete_subscriber(subscriber_);
        if (topic_) participant_->delete_topic(topic_);
        if (participant_) DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    bool init()
    {
        DomainParticipantQos part_qos;
        part_qos.name("QoS_Subscriber");
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, part_qos);
        if (!participant_) return false;

        type_.register_type(participant_);

        // 配置匹配发布者的Topic QoS
        TopicQos topic_qos;
        topic_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
        topic_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        
        topic_ = participant_->create_topic("QoSTopic", type_.get_type_name(), topic_qos);
        if (!topic_) return false;

        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
        if (!subscriber_) return false;

        // 配置DataReader QoS以匹配发布者
        DataReaderQos reader_qos;
        reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
        reader_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        reader_qos.history().kind = KEEP_LAST_HISTORY_QOS;
        reader_qos.history().depth = 10;
        reader_qos.deadline().period = eprosima::fastrtps::Duration_t(1, 0);
        
        reader_ = subscriber_->create_datareader(topic_, reader_qos, &listener_);
        if (!reader_) return false;

        std::cout << "QoS订阅者初始化成功!" << std::endl;
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
                    std::cout << "[" << get_timestamp() << "] 收到: index=" << hello.index()
                              << ", message=\"" << hello.message() << "\"" << std::endl;
                }
            }
        }

        void on_subscription_matched(DataReader*, const SubscriptionMatchedStatus& info) override
        {
            if (info.current_count_change == 1)
                std::cout << "匹配到发布者 (匹配数: " << info.current_count << ")" << std::endl;
        }

        void on_requested_deadline_missed(DataReader*, const RequestedDeadlineMissedStatus& status) override
        {
            std::cout << "警告: 请求的截止时间错过! 总次数: " << status.total_count << std::endl;
        }
    } listener_;
};

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "QoS配置 订阅者示例" << std::endl;
    std::cout << "========================================" << std::endl;

    setup_signal_handler();

    QoSSubscriber sub;
    if (!sub.init()) return 1;

    std::cout << "等待接收数据，按Ctrl+C退出..." << std::endl;
    while (!g_shutdown) sleep_ms(1000);

    return 0;
}