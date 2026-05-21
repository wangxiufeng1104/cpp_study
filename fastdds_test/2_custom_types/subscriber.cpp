/**
 * @file subscriber.cpp
 * @brief 自定义数据类型订阅者示例
 */

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>

#include "Common.hpp"
#include "CustomDataPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;
using namespace fastdds_test;

class CustomSubscriber
{
public:
    CustomSubscriber() : participant_(nullptr), subscriber_(nullptr), topic_(nullptr), reader_(nullptr),
                         type_(new CustomDataPubSubType()) {}

    ~CustomSubscriber()
    {
        if (reader_) subscriber_->delete_datareader(reader_);
        if (subscriber_) participant_->delete_subscriber(subscriber_);
        if (topic_) participant_->delete_topic(topic_);
        if (participant_) DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    bool init()
    {
        DomainParticipantQos pqos;
        pqos.name("CustomData_Subscriber");
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
        if (!participant_) return false;

        type_.register_type(participant_);
        topic_ = participant_->create_topic("CustomDataTopic", type_.get_type_name(), TOPIC_QOS_DEFAULT);
        if (!topic_) return false;

        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
        if (!subscriber_) return false;

        reader_ = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, &listener_);
        if (!reader_) return false;

        std::cout << "自定义数据类型订阅者初始化成功!" << std::endl;
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
            CustomData::DeviceMessage msg;
            while (reader->take_next_sample(&msg, &info) == ReturnCode_t::RETCODE_OK)
            {
                if (info.valid_data)
                {
                    const auto& pos = msg.position();
                    const auto& sensor = msg.sensor();
                    std::cout << "\n[" << get_timestamp() << "] 收到设备消息:" << std::endl;
                    std::cout << "  ID: " << msg.id() << std::endl;
                    std::cout << "  名称: " << msg.device_name() << std::endl;
                    std::cout << "  状态: " << static_cast<int>(msg.status()) << std::endl;
                    std::cout << "  位置: (" << pos.x() << ", " << pos.y() << ", " << pos.z() << ")" << std::endl;
                    std::cout << "  温度: " << sensor.temperature() << "°C" << std::endl;
                    std::cout << "  湿度: " << sensor.humidity() << "%" << std::endl;
                    std::cout << "  数据数组大小: " << msg.data_array().size() << std::endl;
                }
            }
        }

        void on_subscription_matched(DataReader*, const SubscriptionMatchedStatus& info) override
        {
            if (info.current_count_change == 1)
                std::cout << "匹配到发布者" << std::endl;
        }
    } listener_;
};

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "自定义数据类型订阅者示例" << std::endl;
    std::cout << "========================================" << std::endl;

    setup_signal_handler();

    CustomSubscriber sub;
    if (!sub.init()) return 1;

    std::cout << "等待接收数据，按Ctrl+C退出..." << std::endl;
    while (!g_shutdown) sleep_ms(1000);

    return 0;
}