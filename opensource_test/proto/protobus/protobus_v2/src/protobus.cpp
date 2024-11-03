#include "protobus.hpp"
#include <iostream>
#include <stdexcept>
#include "google/protobuf/util/time_util.h"
#include "message.pb.h"
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <unistd.h>
#include <cstdarg>
using namespace std;
using google::protobuf::Timestamp;
using google::protobuf::util::TimeUtil;
std::shared_ptr<protobus> protobus::pinstance_{nullptr};
std::mutex protobus::mutex_;
protobus::protobus(const char *node_name) : log_level(protobus::LOG_DEBUG)
{
    if (node_name != nullptr)
    {
        this->identify = string(node_name);
    }
    run_status = true;

    send_buf = std::make_unique<uint8_t[]>(65535);

    context = new zmq::context_t(2);
    sub_sock = new zmq::socket_t(*context, zmq::socket_type::sub);
    sub_sock->set(zmq::sockopt::rcvhwm, 1500);
    sub_sock->connect(TCP_PUB);
    pub_sock = new zmq::socket_t(*context, zmq::socket_type::pub);
    pub_sock->set(zmq::sockopt::sndhwm, 1500);
    pub_sock->connect(TCP_SUB);
    pub_task = std::thread(&protobus::pub_task_function, this);
    sub_task = std::thread(&protobus::sub_task_function, this);
}

protobus::protobus(const char *node_name, std::vector<std::string> topics, protobus_cb cb) : protobus(node_name)
{
    for (auto it = topics.begin(); it != topics.end(); it++)
    {
        add_subscriber((*it).data(), cb);
    }
}

std::shared_ptr<protobus> protobus::get_instance(const char *node_name)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (pinstance_ == nullptr)
    {
        pinstance_ = std::shared_ptr<protobus>(new protobus(node_name));
    }
    return pinstance_;
}

std::shared_ptr<protobus> protobus::get_instance(const char *node_name, std::vector<std::string> topics, protobus_cb cb)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (pinstance_ == nullptr)
    {
        pinstance_ = std::shared_ptr<protobus>(new protobus(node_name, topics, cb));
    }
    return pinstance_;
}
protobus::~protobus()
{
    run_status = false;

    pub_sock->close();
    sub_sock->close();
    context->shutdown();

    if (sub_task.joinable())
    {
        sub_task.join();
    }
    if (pub_task.joinable())
    {
        pub_task.join();
    }
    std::cout << "exit" << std::endl;
}

void protobus::send(MSG::WrapperMessage &msg)
{
#ifndef THREADSAFE_QUEUE
    std::unique_lock<std::mutex> lk(msg_mutex);
    // Wait until the queue size is below the threshold
    msg_cond.wait(lk, [this]
                  { return msg_queue.size() <= 1000; });
    if (!msg.has_timestamp())
    {
        Timestamp timestamp;
        timestamp.set_seconds(time(NULL));
        timestamp.set_nanos(0);
        *msg.mutable_timestamp() = timestamp;
    }
    msg_queue.push(std::make_shared<MSG::WrapperMessage>(msg));
    msg_cond.notify_one();
#else
    msg_queue.wait_push(std::make_shared<MSG::WrapperMessage>(msg));
#endif
}

void protobus::add_subscriber(const char *topic, protobus_cb cb)
{
    std::lock_guard<std::mutex> lk(topic_mutex);
    string topic_str(topic);

    // Check if the topic already exists
    std::vector<std::pair<std::string, protobus_cb>>::iterator it;
    for (it = topic_vec.begin(); it != topic_vec.end(); it++)
    {
        if ((*it).first == topic_str)
        {
            break;
        }
    }
    if (it == topic_vec.end())
    {
        topic_vec.push_back(std::make_pair(topic_str, cb));
        sub_sock->set(zmq::sockopt::subscribe, topic);
        topic_cond.notify_one();
    }
    else
    {
        std::cerr << "Topic already exists." << std::endl;
    }
}

void protobus::del_subscriber(const char *topic)
{
    std::lock_guard<std::mutex> lk(topic_mutex);
    std::vector<std::pair<std::string, protobus_cb>>::iterator it;
    for (it = topic_vec.begin(); it != topic_vec.end(); ++it)
    {
        if (it->first == topic)
        {
            break;
        }
    }
    if (it != topic_vec.end())
    {
        topic_vec.erase(it);
        std::cout << "topic '" << topic << "' removed from vector." << std::endl;
    }
    else
    {
        std::cout << "topic '" << topic << "' not found in vector." << std::endl;
    }
    sub_sock->set(zmq::sockopt::unsubscribe, topic);
}

int32_t protobus::console(protobus_log_level level, const char *func, int32_t lineNum, const char *format, ...)
{
    if (level < log_level)
        return 0;
    if (NULL == format)
        return -1;
    va_list ap;
    int32_t len = 0;
    std::ostringstream log;
    log << format_timestamp() << "[" << identify << "]" << format_log_level(level);

    if (func)
    {
        log << "[" << func << "]";
    }
    if (lineNum > 0)
    {
        log << "[" << lineNum << "]";
    }
    char buf[65535] = {0};
    va_start(ap, format);
    std::vsnprintf(buf, size(buf), format, ap);
    va_end(ap);

    log << buf;
    std::string log_str = log.str();
    if (!log_str.empty() && log_str.back() == '\n')
    {
        log_str.pop_back();
    }
    std::cout << log_str << std::endl;
    fflush(stdout);

    MSG::WrapperMessage wrapper_msg;
    wrapper_msg.set_topic("log");
    MSG::msg_log *m_log = wrapper_msg.mutable_log();
    m_log->set_log(log_str);
    this->send(wrapper_msg);
    return 0;
}

std::shared_ptr<MSG::WrapperMessage> protobus::get_msg()
{
#ifndef THREADSAFE_QUEUE
    std::unique_lock<mutex> lk(msg_mutex);
    if (!msg_cond.wait_for(lk, std::chrono::milliseconds(500), [this]
                           { return (!msg_queue.empty() || !run_status); }))
    {
        return nullptr;
    }
    if (!run_status)
        return nullptr;
    auto msgPtr = msg_queue.front();
    msg_queue.pop();
    lk.unlock();
    msg_cond.notify_one();
    return msgPtr;
#else
    return msg_queue.wait_and_pop();
#endif
}

size_t protobus::send_msg(std::shared_ptr<MSG::WrapperMessage> msg)
{
    std::unique_ptr<uint8_t[]> dataBuf;
    uint8_t *bufPtr;
    bool releaseFlag = false;
    size_t sendSize = msg->ByteSizeLong();
    if (sendSize > sizeof(this->send_buf))
    {
        dataBuf = std::make_unique<uint8_t[]>(sendSize + 1);
        std::fill(dataBuf.get(), dataBuf.get() + sendSize + 1, 0);
        bufPtr = dataBuf.get();
    }
    else
    {
        bufPtr = this->send_buf.get();
    }

    try
    {
        zmq::message_t topic(msg->topic());
        zmq::send_result_t ret = pub_sock->send(topic, zmq::send_flags::sndmore);
        if (!ret || ret.value() == 0)
        {
            std::cout << "topic send failed" << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << "send proto topic\n";
        return -1;
    }

    try
    {
        Timestamp timestamp;
        timestamp.set_seconds(time(NULL));
        timestamp.set_nanos(0);
        *msg->mutable_timestamp() = timestamp;

        msg->SerializePartialToArray(bufPtr, sendSize);
        zmq::message_t zmq_msg(bufPtr, sendSize);

        zmq::send_result_t ret = pub_sock->send(zmq_msg, zmq::send_flags::dontwait);
        if (!ret || ret.value() == 0)
        {
            std::cout << "message send failed" << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << "send proto message\n";
        return -1;
    }
    return sendSize;
}

void protobus::pub_task_function()
{
    size_t sendSize = 0;

    while (run_status)
    {
        auto msgPtr = get_msg();
        if (msgPtr != nullptr)
        {
            sendSize = send_msg(msgPtr);
            if (sendSize != msgPtr->ByteSizeLong())
            {
                std::cerr << "send msg failed ,ret %d" << sendSize << std::endl;
            }
        }
    }
}
void protobus::sub_task_function()
{
    while (run_status)
    {
        std::unique_lock<mutex> lk(topic_mutex);
        topic_cond.wait(lk, [this]
                        { return !topic_vec.empty(); });
        lk.unlock();
        zmq::message_t zmq_topic;
        zmq::message_t zmq_msg;
        try
        {
            zmq::recv_result_t result = sub_sock->recv(zmq_topic);
            if (result.has_value())
            {

                std::string topic(static_cast<char *>(zmq_topic.data()), zmq_topic.size());
                std::vector<std::pair<std::string, protobus_cb>>::iterator it;
                for (it = topic_vec.begin(); it != topic_vec.end(); ++it)
                {

                    if (topic.find(it->first) != std::string::npos)
                    {
                        break;
                    }
                }
                if (it != topic_vec.end())
                {
                    result = sub_sock->recv(zmq_msg, zmq::recv_flags::none);
                    if (result.has_value())
                    {
                        MSG::WrapperMessage wrapper_msg;
                        wrapper_msg.ParseFromArray(zmq_msg.data(), zmq_msg.size());
                        it->second(wrapper_msg);
                    }
                }
            }
            else
            {
                continue;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }
}

std::string protobus::format_timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct tm *ptm = localtime(&tv.tv_sec);
    char timestamp[64];
    std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", ptm);
    std::ostringstream oss;
    oss << "[" << timestamp << "." << tv.tv_usec % 1000000 << "]";

    return oss.str();
}

std::string protobus::format_log_level(protobus_log_level level)
{
    switch (level)
    {
    case LOG_DEBUG:
        return "[DEBUG]";
    case LOG_INFO:
        return "[INFO]";
    case LOG_WARN:
        return "[WARN]";
    case LOG_ERROR:
        return "[ERROR]";
    default:
        return "[UNKNOWN]";
    }
}
