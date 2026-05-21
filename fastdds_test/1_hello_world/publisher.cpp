/**
 * @file publisher.cpp
 * @brief HelloWorld发布者示例
 * 
 * 演示最基本的FastDDS发布者实现：
 * 1. 创建DomainParticipant
 * 2. 注册数据类型
 * 3. 创建Topic
 * 4. 创建Publisher
 * 5. 创建DataWriter
 * 6. 发布数据
 */

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "Common.hpp"
#include "HelloWorldPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;
using namespace fastdds_test;

// ============================================================================
// DataWriter监听器
// ============================================================================
class HelloWorldPublisherListener : public DataWriterListener
{
public:
    HelloWorldPublisherListener() : matched_(0) {}

    void on_publication_matched(
            DataWriter* writer,
            const PublicationMatchedStatus& info) override
    {
        (void)writer;
        if (info.current_count_change == 1)
        {
            matched_ = static_cast<int16_t>(info.current_count);
            std::cout << "发布者匹配到订阅者: 当前匹配数 = " << matched_ << std::endl;
        }
        else if (info.current_count_change == -1)
        {
            matched_ = static_cast<int16_t>(info.current_count);
            std::cout << "发布者取消匹配订阅者: 当前匹配数 = " << matched_ << std::endl;
        }
    }

    int16_t matched_;

private:
    // 默认构造被禁用，必须通过公共构造
    HelloWorldPublisherListener(const HelloWorldPublisherListener&) = delete;
    HelloWorldPublisherListener& operator=(const HelloWorldPublisherListener&) = delete;
};

// ============================================================================
// HelloWorld发布者类
// ============================================================================
class HelloWorldPublisher
{
public:
    HelloWorldPublisher()
        : participant_(nullptr)
        , publisher_(nullptr)
        , topic_(nullptr)
        , writer_(nullptr)
        , type_(new HelloWorldPubSubType())
        , listener_(new HelloWorldPublisherListener())
    {
    }

    virtual ~HelloWorldPublisher()
    {
        if (writer_)
        {
            publisher_->delete_datawriter(writer_);
        }
        if (publisher_)
        {
            participant_->delete_publisher(publisher_);
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

    //! 初始化发布者
    bool init()
    {
        // 创建DomainParticipant
        DomainParticipantQos participant_qos;
        participant_qos.name("HelloWorld_Publisher");
        
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

        // 创建Publisher
        publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
        if (publisher_ == nullptr)
        {
            std::cerr << "创建Publisher失败" << std::endl;
            return false;
        }

        // 创建DataWriter
        writer_ = publisher_->create_datawriter(
            topic_,
            DATAWRITER_QOS_DEFAULT,
            listener_);
        if (writer_ == nullptr)
        {
            std::cerr << "创建DataWriter失败" << std::endl;
            return false;
        }

        std::cout << "HelloWorld发布者初始化成功!" << std::endl;
        return true;
    }

    //! 发布数据
    bool publish(uint32_t index, const std::string& message)
    {
        HelloWorld hello;
        hello.index(index);
        hello.message(message);
        
        ReturnCode_t ret = writer_->write(&hello);
        if (ret != ReturnCode_t::RETCODE_OK)
        {
            std::cerr << "发布消息失败" << std::endl;
            return false;
        }
        return true;
    }

    //! 检查是否有订阅者
    bool has_subscribers() const
    {
        return listener_->matched_ > 0;
    }

private:
    DomainParticipant* participant_;
    Publisher* publisher_;
    Topic* topic_;
    DataWriter* writer_;
    TypeSupport type_;
    HelloWorldPublisherListener* listener_;
};

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char** argv)
{
    std::cout << "========================================" << std::endl;
    std::cout << "FastDDS HelloWorld 发布者示例" << std::endl;
    std::cout << "========================================" << std::endl;

    // 设置信号处理
    setup_signal_handler();

    // 创建发布者
    HelloWorldPublisher publisher;
    if (!publisher.init())
    {
        return 1;
    }

    // 等待订阅者连接
    std::cout << "等待订阅者连接..." << std::endl;
    while (!publisher.has_subscribers() && !g_shutdown)
    {
        sleep_ms(100);
    }

    // 发布消息
    uint32_t index = 0;
    while (!g_shutdown)
    {
        std::string message = "Hello World! #" + std::to_string(index);
        
        if (publisher.publish(index, message))
        {
            std::cout << "[" << get_timestamp() << "] "
                      << "已发布: index=" << index 
                      << ", message=\"" << message << "\"" << std::endl;
        }
        
        index++;
        sleep_ms(1000);  // 每秒发布一条消息
    }

    std::cout << "发布者退出" << std::endl;
    return 0;
}