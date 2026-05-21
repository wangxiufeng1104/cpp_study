/**
 * @file publisher.cpp
 * @brief 自定义数据类型发布者示例
 * 
 * 演示如何使用复杂的数据类型，包括：
 * - 枚举类型
 * - 嵌套结构体
 * - 序列类型
 */

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include "Common.hpp"
#include "CustomDataPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;
using namespace fastdds_test;

class CustomPublisher
{
public:
    CustomPublisher() : participant_(nullptr), publisher_(nullptr), topic_(nullptr), writer_(nullptr),
                        type_(new CustomDataPubSubType()), matched_(0) {}

    ~CustomPublisher()
    {
        if (writer_) publisher_->delete_datawriter(writer_);
        if (publisher_) participant_->delete_publisher(publisher_);
        if (topic_) participant_->delete_topic(topic_);
        if (participant_) DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    bool init()
    {
        DomainParticipantQos pqos;
        pqos.name("CustomData_Publisher");
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
        if (!participant_) return false;

        type_.register_type(participant_);
        topic_ = participant_->create_topic("CustomDataTopic", type_.get_type_name(), TOPIC_QOS_DEFAULT);
        if (!topic_) return false;

        publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
        if (!publisher_) return false;

        writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, &listener_);
        if (!writer_) return false;

        std::cout << "自定义数据类型发布者初始化成功!" << std::endl;
        return true;
    }

    bool wait_for_subscribers(int timeout_ms = 5000)
    {
        int waited = 0;
        while (matched_ == 0 && waited < timeout_ms)
        {
            sleep_ms(100);
            waited += 100;
        }
        return matched_ > 0;
    }

    void publish()
    {
        CustomData::DeviceMessage msg;
        msg.id(1);
        msg.device_name("SensorDevice_001");
        msg.status(CustomData::STATUS_RUNNING);
        
        // 设置位置
        CustomData::Position pos;
        pos.x(10.5);
        pos.y(20.3);
        pos.z(5.1);
        msg.position(pos);
        
        // 设置速度
        CustomData::Velocity vel;
        vel.vx(1.2);
        vel.vy(3.4);
        vel.vz(0.5);
        msg.velocity(vel);
        
        // 设置传感器数据
        CustomData::SensorData sensor;
        sensor.temperature(25.6);
        sensor.humidity(65.2);
        sensor.pressure(1013.25);
        msg.sensor(sensor);
        
        // 设置数据数组
        std::vector<int64_t> arr = {100, 200, 300, 400, 500};
        msg.data_array(arr);
        
        // 设置时间戳
        msg.timestamp(std::chrono::system_clock::now().time_since_epoch().count());
        
        if (writer_->write(&msg) == ReturnCode_t::RETCODE_OK)
        {
            std::cout << "[" << get_timestamp() << "] 已发布设备消息:" << std::endl;
            std::cout << "  ID: " << msg.id() << std::endl;
            std::cout << "  名称: " << msg.device_name() << std::endl;
            std::cout << "  状态: " << static_cast<int>(msg.status()) << std::endl;
            std::cout << "  位置: (" << pos.x() << ", " << pos.y() << ", " << pos.z() << ")" << std::endl;
            std::cout << "  温度: " << sensor.temperature() << "°C" << std::endl;
        }
    }

private:
    DomainParticipant* participant_;
    Publisher* publisher_;
    Topic* topic_;
    DataWriter* writer_;
    TypeSupport type_;
    int matched_;

    class PubListener : public DataWriterListener
    {
    public:
        void on_publication_matched(DataWriter*, const PublicationMatchedStatus& info) override
        {
            if (info.current_count_change == 1)
                std::cout << "匹配到订阅者" << std::endl;
            else if (info.current_count_change == -1)
                std::cout << "取消匹配订阅者" << std::endl;
        }
    } listener_;
};

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "自定义数据类型发布者示例" << std::endl;
    std::cout << "========================================" << std::endl;

    setup_signal_handler();

    CustomPublisher pub;
    if (!pub.init()) return 1;

    std::cout << "等待订阅者..." << std::endl;
    pub.wait_for_subscribers();

    while (!g_shutdown)
    {
        pub.publish();
        sleep_ms(2000);
    }

    return 0;
}