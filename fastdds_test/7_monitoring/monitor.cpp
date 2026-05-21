/**
 * @file monitor.cpp
 * @brief 实时统计/监控示例
 * 
 * 演示如何使用FastDDS的统计和监控功能：
 * - 获取DataReader统计信息
 * - 监控数据吞吐量
 * - 显示连接状态
 */

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <iomanip>
#include <sstream>

#include "Common.hpp"
#include "HelloWorldPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;
using namespace fastdds_test;

class Monitor
{
public:
    Monitor() : participant_(nullptr), subscriber_(nullptr), topic_(nullptr), 
                reader_(nullptr), type_(new HelloWorldPubSubType()),
                total_samples_(0), matched_count_(0) {}

    ~Monitor()
    {
        if (reader_) subscriber_->delete_datareader(reader_);
        if (subscriber_) participant_->delete_subscriber(subscriber_);
        if (topic_) participant_->delete_topic(topic_);
        if (participant_) DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    bool init()
    {
        DomainParticipantQos pqos;
        pqos.name("Monitor_Participant");
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
        if (!participant_) return false;

        type_.register_type(participant_);

        topic_ = participant_->create_topic("MonitorTopic", type_.get_type_name(), TOPIC_QOS_DEFAULT);
        if (!topic_) return false;

        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
        if (!subscriber_) return false;

        DataReaderQos rqos;
        rqos.history().kind = KEEP_ALL_HISTORY_QOS;

        reader_ = subscriber_->create_datareader(topic_, rqos, &listener_);
        if (!reader_) return false;

        std::cout << "监控初始化成功!" << std::endl;
        return true;
    }

    void run()
    {
        std::cout << "\n等待数据，按Ctrl+C退出..." << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        
        auto last_count = total_samples_;
        auto last_time = std::chrono::steady_clock::now();
        
        while (!g_shutdown)
        {
            sleep_ms(1000);
            
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count();
            
            if (elapsed > 0 && total_samples_ > last_count)
            {
                double rate = (total_samples_ - last_count) * 1000.0 / elapsed;
                
                std::cout << "\r" << std::string(80, ' ') << "\r";
                std::cout << "[" << get_timestamp() << "] "
                          << "总样本: " << std::setw(6) << total_samples_
                          << " | 速率: " << std::setw(6) << std::fixed << std::setprecision(1) << rate
                          << " samples/s | 匹配: " << matched_count_
                          << std::flush;
                
                last_count = total_samples_;
                last_time = now;
            }
        }
        std::cout << std::endl;
    }

private:
    DomainParticipant* participant_;
    Subscriber* subscriber_;
    Topic* topic_;
    DataReader* reader_;
    TypeSupport type_;
    std::atomic<uint64_t> total_samples_;
    std::atomic<int16_t> matched_count_;

    class MonitorListener : public DataReaderListener
    {
    public:
        MonitorListener(Monitor* parent) : parent_(parent) {}

        void on_data_available(DataReader* reader) override
        {
            SampleInfo info;
            HelloWorld hello;
            while (reader->take_next_sample(&hello, &info) == ReturnCode_t::RETCODE_OK)
            {
                if (info.valid_data)
                {
                    parent_->total_samples_++;
                }
            }
        }

        void on_subscription_matched(DataReader*, const SubscriptionMatchedStatus& info) override
        {
            parent_->matched_count_ = static_cast<int16_t>(info.current_count);
            if (info.current_count_change == 1)
                std::cout << "\n[事件] 匹配到发布者 (匹配数: " << info.current_count << ")" << std::endl;
            else if (info.current_count_change == -1)
                std::cout << "\n[事件] 取消匹配发布者 (匹配数: " << info.current_count << ")" << std::endl;
        }

    private:
        Monitor* parent_;
    } listener_{this};
};

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "实时监控示例" << std::endl;
    std::cout << "========================================" << std::endl;

    setup_signal_handler();

    Monitor monitor;
    if (!monitor.init()) return 1;

    monitor.run();

    std::cout << "\n监控退出" << std::endl;
    return 0;
}