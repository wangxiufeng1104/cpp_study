/**
 * @file subscriber.cpp
 * @brief HelloWorld订阅者示例
 * 
 * 演示最基本的FastDDS订阅者实现：
 * 1. 创建DomainParticipant
 * 2. 注册数据类型
 * 3. 创建Topic
 * 4. 创建Subscriber
 * 5. 创建DataReader
 * 6. 接收数据
 */

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "Common.hpp"
#include "HelloWorldPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;
using namespace fastdds_test;

// ============================================================================
// DataReader监听器
// ============================================================================
class HelloWorldSubscriberListener : public DataReaderListener
{
public:
    HelloWorldSubscriberListener() : samples_(0) {}

    void on_subscription_matched(
            DataReader* reader,
            const SubscriptionMatchedStatus& info) override
    {
        (void)reader;
        if (info.current_count_change == 1)
        {
            std::cout << "订阅者匹配到发布者: 当前匹配数 = " << info.current_count << std::endl;
        }
        else if (info.current_count_change == -1)
        {
            std::cout << "订阅者取消匹配发布者: 当前匹配数 = " << info.current_count << std::endl;
        }
    }

    void on_data_available(
            DataReader* reader) override
    {
        SampleInfo info;
        HelloWorld hello;
        
        while (reader->take_next_sample(&hello, &info) == ReturnCode_t::RETCODE_OK)
        {
            if (info.valid_data)
            {
                samples_++;
                std::cout << "[" << get_timestamp() << "] "
                          << "收到消息: index=" << hello.index()
                          << ", message=\"" << hello.message() << "\""
                          << " (总计: " << samples_ << " 条)" << std::endl;
            }
        }
    }

    int samples_;

private:
    HelloWorldSubscriberListener(const HelloWorldSubscriberListener&) = delete;
    HelloWorldSubscriberListener& operator=(const HelloWorldSubscriberListener&) = delete;
};

// ============================================================================
// HelloWorld订阅者类
// ============================================================================
class HelloWorldSubscriber
{
public:
    HelloWorldSubscriber()
        : participant_(nullptr)
        , subscriber_(nullptr)
        , topic_(nullptr)
        , reader_(nullptr)
        , type_(new HelloWorldPubSubType())
        , listener_(new HelloWorldSubscriberListener())
    {
    }

    virtual ~HelloWorldSubscriber()
    {
        if (reader_)
        {
            subscriber_->delete_datareader(reader_);
        }
        if (subscriber_)
        {
            participant_->delete_subscriber(subscriber_);
        }
        if (topic_)
        {
            participant_->delete_topic(topic_);
        }
        if (participant_)
        {
            DomainParticipantFactory::get_instance()->delete_participant(participant_);
        }
        delete listener_;
    }

    //! 初始化订阅者
    bool init()
    {
        // 创建DomainParticipant
        DomainParticipantQos participant_qos;
        participant_qos.name("HelloWorld_Subscriber");
        
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, participant_qos);
        if (participant_ == nullptr)
        {
            std::cerr << "创建DomainParticipant失败" << std::endl;
            return false;
        }

        // 注册数据类型
        type_.register_type(participant_);

        // 创建Topic
        topic_ = participant_->create_topic(
            "HelloWorldTopic",
            type_.get_type_name(),
            TOPIC_QOS_DEFAULT);
        if (topic_ == nullptr)
        {
            std::cerr << "创建Topic失败" << std::endl;
            return false;
        }

        // 创建Subscriber
        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
        if (subscriber_ == nullptr)
        {
            std::cerr << "创建Subscriber失败" << std::endl;
            return false;
        }

        // 创建DataReader
        reader_ = subscriber_->create_datareader(
            topic_,
            DATAREADER_QOS_DEFAULT,
            listener_);
        if (reader_ == nullptr)
        {
            std::cerr << "创建DataReader失败" << std::endl;
            return false;
        }

        std::cout << "HelloWorld订阅者初始化成功!" << std::endl;
        return true;
    }

private:
    DomainParticipant* participant_;
    Subscriber* subscriber_;
    Topic* topic_;
    DataReader* reader_;
    TypeSupport type_;
    HelloWorldSubscriberListener* listener_;
};

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char** argv)
{
    std::cout << "========================================" << std::endl;
    std::cout << "FastDDS HelloWorld 订阅者示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // 设置信号处理
    setup_signal_handler();

    // 创建订阅者
    HelloWorldSubscriber subscriber;
    if (!subscriber.init())
    {
        return 1;
    }

    std::cout << "等待接收数据，按Ctrl+C退出..." << std::endl;
    
    // 等待退出信号
    while (!g_shutdown)
    {
        sleep_ms(1000);
    }

    std::cout << "订阅者退出" << std::endl;
    return 0;
}