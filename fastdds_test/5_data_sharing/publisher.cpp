/**
 * @file publisher.cpp
 * @brief DataSharing共享内存通信发布者示例
 * 
 * 演示FastDDS的DataSharing特性：
 * - 使用共享内存进行高效数据传输
 * - 避免数据序列化/反序列化开销
 * - 适用于同主机高吞吐量场景
 */

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/rtps/attributes/PropertyParser.h>

#include "Common.hpp"
#include "HelloWorldPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;
using namespace fastdds_test;

class DataSharingPublisher
{
public:
    DataSharingPublisher() : participant_(nullptr), publisher_(nullptr), topic_(nullptr), 
                              writer_(nullptr), type_(new HelloWorldPubSubType()) {}

    ~DataSharingPublisher()
    {
        if (writer_) publisher_->delete_datawriter(writer_);
        if (publisher_) participant_->delete_publisher(publisher_);
        if (topic_) participant_->delete_topic(topic_);
        if (participant_) DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    bool init()
    {
        // 创建DomainParticipant
        DomainParticipantQos part_qos;
        part_qos.name("DataSharing_Publisher");
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, part_qos);
        if (!participant_) return false;

        type_.register_type(participant_);

        // 配置Topic QoS - 启用DataSharing
        TopicQos topic_qos;
        topic_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
        topic_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        topic_qos.history().kind = KEEP_LAST_HISTORY_QOS;
        topic_qos.history().depth = 10;
        
        // DataSharing配置
        topic_qos.data_sharing().automatic();
        // 也可以指定共享内存目录
        // topic_qos.data_sharing().directories.push_back("/dev/shm/fastdds_datasharing");
        topic_qos.data_sharing().max_domains = 1;
        topic_qos.data_sharing().max_participants_per_domain = 10;

        topic_ = participant_->create_topic("DataSharingTopic", type_.get_type_name(), topic_qos);
        if (!topic_) return false;

        publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
        if (!publisher_) return false;

        // 配置DataWriter QoS - DataSharing
        DataWriterQos writer_qos;
        writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
        writer_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        writer_qos.history().kind = KEEP_LAST_HISTORY_QOS;
        writer_qos.history().depth = 10;
        
        // 启用DataSharing
        writer_qos.data_sharing().automatic();

        writer_ = publisher_->create_datawriter(topic_, writer_qos, &listener_);
        if (!writer_) return false;

        std::cout << "DataSharing发布者初始化成功!" << std::endl;
        std::cout << "DataSharing配置: AUTOMATIC" << std::endl;
        std::cout << "说明: 使用共享内存进行高效数据传输" << std::endl;
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
                std::cout << "匹配到DataSharing订阅者 (匹配数: " << info.current_count << ")" << std::endl;
        }
    } listener_;
};

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "DataSharing共享内存 发布者示例" << std::endl;
    std::cout << "========================================" << std::endl;

    setup_signal_handler();

    DataSharingPublisher pub;
    if (!pub.init()) return 1;

    std::cout << "\n等待订阅者连接..." << std::endl;
    sleep_ms(2000);

    uint32_t index = 0;
    while (!g_shutdown)
    {
        std::string msg = "DataSharing Message #" + std::to_string(index);
        if (pub.publish(index, msg))
        {
            std::cout << "[" << get_timestamp() << "] 已发布(共享内存): " << msg << std::endl;
        }
        index++;
        sleep_ms(500);  // 高频发送测试
    }

    return 0;
}