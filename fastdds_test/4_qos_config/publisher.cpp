/**
 * @file publisher.cpp
 * @brief QoS配置发布者示例
 * 
 * 演示各种QoS策略配置：
 * - Reliability (可靠性)
 * - Durability (持久性)
 * - Deadline (截止时间)
 * - Lifespan (生命周期)
 * - History (历史深度)
 */

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

#include "Common.hpp"
#include "HelloWorldPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;
using namespace fastdds_test;

class QoSPublisher
{
public:
    QoSPublisher() : participant_(nullptr), publisher_(nullptr), topic_(nullptr), writer_(nullptr),
                     type_(new HelloWorldPubSubType()) {}

    ~QoSPublisher()
    {
        if (writer_) publisher_->delete_datawriter(writer_);
        if (publisher_) participant_->delete_publisher(publisher_);
        if (topic_) participant_->delete_topic(topic_);
        if (participant_) DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    bool init()
    {
        // 1. 创建DomainParticipant
        DomainParticipantQos part_qos;
        part_qos.name("QoS_Publisher");
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, part_qos);
        if (!participant_) return false;

        type_.register_type(participant_);

        // 2. 配置Topic QoS
        TopicQos topic_qos;
        topic_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;  // 可靠传输
        topic_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;  // 本地持久化
        topic_qos.durability_service().history_depth = 10;  // 历史深度
        
        topic_ = participant_->create_topic(
            "QoSTopic",
            type_.get_type_name(),
            topic_qos);
        if (!topic_) return false;

        // 3. 配置Publisher QoS
        PublisherQos pub_qos;
        pub_qos.presentation().access_scope = INSTANCE_PRESENTATION_QOS;
        pub_qos.presentation().coherent_access = false;
        pub_qos.presentation().ordered_access = true;
        
        publisher_ = participant_->create_publisher(pub_qos, nullptr);
        if (!publisher_) return false;

        // 4. 配置DataWriter QoS
        DataWriterQos writer_qos;
        
        // 可靠性: 可靠传输
        writer_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
        
        // 持久性: 订阅者连接后可以获取之前发送的数据
        writer_qos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        
        // 历史深度: 保留最近10条消息
        writer_qos.history().kind = KEEP_LAST_HISTORY_QOS;
        writer_qos.history().depth = 10;
        
        // 截止时间: 每1000ms必须发送一条消息
        writer_qos.deadline().period = eprosima::fastrtps::Duration_t(1, 0);
        
        // 生命周期: 数据在5000ms后过期
        writer_qos.lifespan().duration = eprosima::fastrtps::Duration_t(5, 0);
        
        writer_ = publisher_->create_datawriter(topic_, writer_qos, &listener_);
        if (!writer_) return false;

        std::cout << "QoS发布者初始化成功!" << std::endl;
        std::cout << "QoS配置:" << std::endl;
        std::cout << "  Reliability: RELIABLE" << std::endl;
        std::cout << "  Durability: TRANSIENT_LOCAL" << std::endl;
        std::cout << "  History: KEEP_LAST (depth=10)" << std::endl;
        std::cout << "  Deadline: 1000ms" << std::endl;
        std::cout << "  Lifespan: 5000ms" << std::endl;
        
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
                std::cout << "匹配到订阅者 (匹配数: " << info.current_count << ")" << std::endl;
            else if (info.current_count_change == -1)
                std::cout << "取消匹配订阅者 (匹配数: " << info.current_count << ")" << std::endl;
        }

        void on_offered_deadline_missed(DataWriter*, const OfferedDeadlineMissedStatus& status) override
        {
            std::cout << "警告: 错过截止时间! 总次数: " << status.total_count << std::endl;
        }
    } listener_;
};

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "QoS配置 发布者示例" << std::endl;
    std::cout << "========================================" << std::endl;

    setup_signal_handler();

    QoSPublisher pub;
    if (!pub.init()) return 1;

    std::cout << "\n等待订阅者连接..." << std::endl;
    sleep_ms(2000);

    uint32_t index = 0;
    while (!g_shutdown)
    {
        std::string msg = "QoS Message #" + std::to_string(index);
        if (pub.publish(index, msg))
        {
            std::cout << "[" << get_timestamp() << "] 已发布: " << msg << std::endl;
        }
        index++;
        sleep_ms(1000);  // 满足deadline要求
    }

    return 0;
}