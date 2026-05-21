/**
 * @file discovery_participant.cpp
 * @brief 发现机制示例
 * 
 * 演示FastDDS的参与者发现机制：
 * - 监控参与者发现
 * - 监控主题发现
 * - 监控端点发现
 */

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>

#include "Common.hpp"

using namespace eprosima::fastdds::dds;
using namespace fastdds_test;

class DiscoveryMonitor
{
public:
    DiscoveryMonitor() : participant_(nullptr) {}

    ~DiscoveryMonitor()
    {
        if (participant_)
            DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    bool init()
    {
        DomainParticipantQos pqos;
        pqos.name("Discovery_Monitor");
        
        participant_ = DomainParticipantFactory::get_instance()->create_participant(
            0, pqos, &listener_);
        
        if (!participant_) return false;

        std::cout << "发现监控初始化成功!" << std::endl;
        return true;
    }

    void run()
    {
        std::cout << "\n监控网络参与者..." << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        
        while (!g_shutdown)
        {
            sleep_ms(1000);
        }
    }

private:
    DomainParticipant* participant_;

    class DiscoveryListener : public DomainParticipantListener
    {
    public:
        void on_participant_discovery(
            DomainParticipant*,
            ParticipantDiscoveryStatus status,
            const ParticipantBuiltinTopicData& info,
            bool&) override
        {
            switch (status)
            {
                case ParticipantDiscoveryStatus::DISCOVERED:
                    std::cout << "\n[发现参与者] " << info.participant_name.to_string() 
                              << " (GUID: " << info.guid << ")" << std::endl;
                    break;
                case ParticipantDiscoveryStatus::REMOVED:
                    std::cout << "\n[移除参与者] GUID: " << info.guid << std::endl;
                    break;
                case ParticipantDiscoveryStatus::DROPPED:
                    std::cout << "\n[丢失参与者] GUID: " << info.guid << std::endl;
                    break;
                default:
                    break;
            }
        }

        void on_subscriber_discovery(
            DomainParticipant*,
            ReaderDiscoveryStatus status,
            const SubscriptionBuiltinTopicData& info,
            bool&) override
        {
            switch (status)
            {
                case ReaderDiscoveryStatus::DISCOVERED:
                    std::cout << "\n[发现订阅者] Topic: " << info.topic_name.to_string()
                              << ", Type: " << info.type_name.to_string() << std::endl;
                    break;
                case ReaderDiscoveryStatus::REMOVED:
                    std::cout << "\n[移除订阅者] Topic: " << info.topic_name.to_string() << std::endl;
                    break;
                default:
                    break;
            }
        }

        void on_publisher_discovery(
            DomainParticipant*,
            WriterDiscoveryStatus status,
            const PublicationBuiltinTopicData& info,
            bool&) override
        {
            switch (status)
            {
                case WriterDiscoveryStatus::DISCOVERED:
                    std::cout << "\n[发现发布者] Topic: " << info.topic_name.to_string()
                              << ", Type: " << info.type_name.to_string() << std::endl;
                    break;
                case WriterDiscoveryStatus::REMOVED:
                    std::cout << "\n[移除发布者] Topic: " << info.topic_name.to_string() << std::endl;
                    break;
                default:
                    break;
            }
        }
    } listener_;
};

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "发现机制示例" << std::endl;
    std::cout << "========================================" << std::endl;

    setup_signal_handler();

    DiscoveryMonitor monitor;
    if (!monitor.init()) return 1;

    monitor.run();

    std::cout << "\n发现监控退出" << std::endl;
    return 0;
}