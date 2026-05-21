/**
 * @file subscriber.cpp
 * @brief 安全机制订阅者示例
 */

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/rtps/attributes/PropertyParser.h>

#include "Common.hpp"
#include "HelloWorldPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;
using namespace fastdds_test;

class SecureSubscriber
{
public:
    SecureSubscriber() : participant_(nullptr), subscriber_(nullptr), topic_(nullptr), 
                         reader_(nullptr), type_(new HelloWorldPubSubType()) {}

    ~SecureSubscriber()
    {
        if (reader_) subscriber_->delete_datareader(reader_);
        if (subscriber_) participant_->delete_subscriber(subscriber_);
        if (topic_) participant_->delete_topic(topic_);
        if (participant_) DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    bool init(const std::string& certs_path)
    {
        DomainParticipantQos pqos;
        pqos.name("Secure_Subscriber");

        // 配置DDS Security
        pqos.properties().properties().emplace_back(
            "dds.sec.auth.plugin", "builtin.PKI-DH");
        pqos.properties().properties().emplace_back(
            "dds.sec.auth.builtin.PKI-DH.identity_ca", 
            "file:" + certs_path + "/cacert.pem");
        pqos.properties().properties().emplace_back(
            "dds.sec.auth.builtin.PKI-DH.identity_certificate",
            "file:" + certs_path + "/cert.pem");
        pqos.properties().properties().emplace_back(
            "dds.sec.auth.builtin.PKI-DH.private_key",
            "file:" + certs_path + "/key.pem");

        pqos.properties().properties().emplace_back(
            "dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");

        pqos.properties().properties().emplace_back(
            "dds.sec.access.plugin", "builtin.Access-Permissions");
        pqos.properties().properties().emplace_back(
            "dds.sec.access.builtin.Access-Permissions.permissions_ca",
            "file:" + certs_path + "/cacert.pem");
        pqos.properties().properties().emplace_back(
            "dds.sec.access.builtin.Access-Permissions.governance",
            "file:" + certs_path + "/governance.p7s");
        pqos.properties().properties().emplace_back(
            "dds.sec.access.builtin.Access-Permissions.permissions",
            "file:" + certs_path + "/permissions.p7s");

        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
        if (!participant_)
        {
            std::cerr << "错误: 无法创建安全参与者 (检查证书配置)" << std::endl;
            return false;
        }

        type_.register_type(participant_);

        topic_ = participant_->create_topic("SecureTopic", type_.get_type_name(), TOPIC_QOS_DEFAULT);
        if (!topic_) return false;

        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
        if (!subscriber_) return false;

        reader_ = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, &listener_);
        if (!reader_) return false;

        std::cout << "安全订阅者初始化成功!" << std::endl;
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
                    std::cout << "[" << get_timestamp() << "] 收到(解密): index=" 
                              << hello.index() << ", message=\"" << hello.message() << "\"" << std::endl;
                }
            }
        }

        void on_subscription_matched(DataReader*, const SubscriptionMatchedStatus& info) override
        {
            if (info.current_count_change == 1)
                std::cout << "匹配到安全发布者 (匹配数: " << info.current_count << ")" << std::endl;
        }
    } listener_;
};

int main(int argc, char** argv)
{
    std::cout << "========================================" << std::endl;
    std::cout << "安全机制 订阅者示例" << std::endl;
    std::cout << "========================================" << std::endl;

    setup_signal_handler();

    std::string certs_path = "./certs";
    if (argc > 1) certs_path = argv[1];

    SecureSubscriber sub;
    if (!sub.init(certs_path))
    {
        std::cout << "\n提示: 安全机制需要正确的证书文件。" << std::endl;
        return 1;
    }

    std::cout << "等待接收加密数据，按Ctrl+C退出..." << std::endl;
    while (!g_shutdown) sleep_ms(1000);

    return 0;
}