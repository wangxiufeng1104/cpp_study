/**
 * @file publisher.cpp
 * @brief 安全机制发布者示例
 * 
 * 演示FastDDS的安全特性：
 * - DDS Security配置
 * - 身份验证
 * - 加密通信
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

class SecurePublisher
{
public:
    SecurePublisher() : participant_(nullptr), publisher_(nullptr), topic_(nullptr), 
                        writer_(nullptr), type_(new HelloWorldPubSubType()) {}

    ~SecurePublisher()
    {
        if (writer_) publisher_->delete_datawriter(writer_);
        if (publisher_) participant_->delete_publisher(publisher_);
        if (topic_) participant_->delete_topic(topic_);
        if (participant_) DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    bool init(const std::string& certs_path)
    {
        DomainParticipantQos pqos;
        pqos.name("Secure_Publisher");

        // 配置DDS Security
        // 注意: 需要正确的证书文件才能启用安全
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

        publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
        if (!publisher_) return false;

        writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, &listener_);
        if (!writer_) return false;

        std::cout << "安全发布者初始化成功!" << std::endl;
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
                std::cout << "匹配到安全订阅者 (匹配数: " << info.current_count << ")" << std::endl;
        }
    } listener_;
};

int main(int argc, char** argv)
{
    std::cout << "========================================" << std::endl;
    std::cout << "安全机制 发布者示例" << std::endl;
    std::cout << "========================================" << std::endl;

    setup_signal_handler();

    std::string certs_path = "./certs";
    if (argc > 1) certs_path = argv[1];

    SecurePublisher pub;
    if (!pub.init(certs_path))
    {
        std::cout << "\n提示: 安全机制需要正确的证书文件。" << std::endl;
        std::cout << "请确保以下文件存在:" << std::endl;
        std::cout << "  - cacert.pem (CA证书)" << std::endl;
        std::cout << "  - cert.pem (身份证书)" << std::endl;
        std::cout << "  - key.pem (私钥)" << std::endl;
        std::cout << "  - governance.p7s (治理规则)" << std::endl;
        std::cout << "  - permissions.p7s (权限规则)" << std::endl;
        return 1;
    }

    std::cout << "\n等待安全订阅者连接..." << std::endl;
    sleep_ms(2000);

    uint32_t index = 0;
    while (!g_shutdown)
    {
        std::string msg = "Secure Message #" + std::to_string(index);
        if (pub.publish(index, msg))
        {
            std::cout << "[" << get_timestamp() << "] 已发布(加密): " << msg << std::endl;
        }
        index++;
        sleep_ms(1000);
    }

    return 0;
}