#include "protobus.hpp"
#include <iostream>
#include "zmq/zmq.hpp"
#include <stdexcept>
#include "google/protobuf/util/time_util.h"
#include "message.pb.h"
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <unistd.h>
#include <thread>
#include <condition_variable>
#include <queue>
using namespace std;
using google::protobuf::Timestamp;
using google::protobuf::util::TimeUtil;
// #define TOPIC_MAP
uint8_t sendBuf[65535];
struct protobus_handle
{
    /* pub socket */
    zmq::socket_t *pub;
    /* sub socket */
    zmq::socket_t *sub;
    /* sub ctx */
    zmq::context_t *sub_ctx;
    /* sender identify*/
    string identify;
    /* run status */
    bool isRunning = false;
    /* topic */
    std::mutex topicMutex;
    std::condition_variable topicCond;
#ifdef TOPIC_MAP
    /* topic map */
    std::unordered_map<string, protobus_cb> topicMap;
#else
    std::vector<std::pair<string, protobus_cb>> topicVec;
#endif
    /* protobuf msg */
    std::queue<std::shared_ptr<MSG::WrapperMessage>> msgQueue;
    std::mutex queueMutex;
    std::condition_variable queueCond;
};
static size_t send_msg(protobus_handle_t *handle, std::shared_ptr<MSG::WrapperMessage> msg)
{
    std::unique_ptr<uint8_t[]> dataBuf;
    uint8_t *bufPtr;
    bool releaseFlag = false;
    size_t sendSize = msg->ByteSizeLong();
    if (sendSize > sizeof(sendBuf))
    {
        dataBuf = std::make_unique<uint8_t[]>(sendSize + 1);
        std::fill(dataBuf.get(), dataBuf.get() + sendSize + 1, 0);
        bufPtr = dataBuf.get();
    }
    else
    {
        bufPtr = sendBuf;
    }

    try
    {
        zmq::message_t topic(msg->topic());
        zmq::send_result_t ret = handle->pub->send(topic, zmq::send_flags::sndmore);
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

        zmq::send_result_t ret = handle->pub->send(zmq_msg, zmq::send_flags::dontwait);
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
static std::shared_ptr<MSG::WrapperMessage> get_msg(protobus_handle_t *handle)
{
    std::unique_lock<mutex> lock(handle->queueMutex);
    handle->queueCond.wait(lock, [&handle]
                           { return !handle->msgQueue.empty(); });
    auto msgPtr = handle->msgQueue.front();
    handle->msgQueue.pop();
    lock.unlock();
    handle->queueCond.notify_one(); // 通知等待的生产者线程
    return msgPtr;
}
static void sub_task(protobus_handle_t *handle)
{
    while (handle->isRunning)
    {
        std::unique_lock<mutex> lock(handle->topicMutex);
#ifdef TOPIC_MAP
        handle->topicCond.wait(lock, [&handle]
                               { return !handle->topicMap.empty(); });
#else
        handle->topicCond.wait(lock, [&handle]
                               { return !handle->topicVec.empty(); });
#endif
        lock.unlock();

        zmq::message_t zmq_topic;
        zmq::message_t zmq_msg;

        zmq::recv_result_t result = handle->sub->recv(zmq_topic);
        if (result.has_value())
        {

            std::string topic(static_cast<char *>(zmq_topic.data()), zmq_topic.size());
#ifdef TOPIC_MAP
            auto it = handle->topicMap.find(topic);
            if (it != handle->topicMap.end())
            {
                result = handle->sub->recv(zmq_msg, zmq::recv_flags::none);
                if (result.has_value())
                {
                    MSG::WrapperMessage wrapper_msg;
                    wrapper_msg.ParseFromArray(zmq_msg.data(), zmq_msg.size());
                    it->second(wrapper_msg);
                }
            }
#else
            std::vector<std::pair<std::string, protobus_cb>>::iterator it;
            for (it = handle->topicVec.begin(); it != handle->topicVec.end(); ++it)
            {

                if (topic.find(it->first) != std::string::npos)
                {
                    break;
                }
            }
            if (it != handle->topicVec.end())
            {
                result = handle->sub->recv(zmq_msg, zmq::recv_flags::none);
                if (result.has_value())
                {
                    MSG::WrapperMessage wrapper_msg;
                    wrapper_msg.ParseFromArray(zmq_msg.data(), zmq_msg.size());
                    it->second(wrapper_msg);
                }
            }
#endif
        }
        else
        {
            continue;
        }
    }
    handle->sub->close();
    handle->sub = nullptr;
}
static void pub_task(protobus_handle_t *handle)
{
    size_t sendSize = 0;
    int snd_hwm = 1500;
    zmq::context_t ctx(1);
    handle->pub = new zmq::socket_t(ctx, zmq::socket_type::pub);
    handle->pub->set(zmq::sockopt::sndhwm, 1500);
    handle->pub->connect(TCP_SUB);

    while (handle->isRunning)
    {
        auto msgPtr = get_msg(handle);
        if (msgPtr != nullptr)
        {
            sendSize = send_msg(handle, msgPtr);
            if (sendSize != msgPtr->ByteSizeLong())
            {
                std::cerr << "send msg failed ,ret %d" << sendSize << std::endl;
            }
        }
    }
}
static void subscriber_init(protobus_handle_t *handle)
{
    int rcv_hwm = 1500;
    handle->sub_ctx = new zmq::context_t(1);
    handle->sub = new zmq::socket_t(*handle->sub_ctx, zmq::socket_type::sub);
    handle->sub->set(zmq::sockopt::rcvhwm, rcv_hwm);
    handle->sub->connect(TCP_PUB);
    thread t(sub_task, handle);
    t.detach();
}
protobus_handle_t *protobus_init(const char *node_name)
{
    protobus_handle_t *handle = nullptr;
    try
    {
        handle = new protobus_handle_t();

        handle->pub = nullptr;
        handle->sub = nullptr;

        if (node_name != nullptr)
        {
            handle->identify = string(node_name);
        }
        handle->isRunning = true;
        thread t(pub_task, handle);
        t.detach();
        subscriber_init(handle);
    }
    catch (const std::exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        delete handle;
        handle = nullptr;
    }
    return handle;
}
protobus_handle_t *protobus_init(const char *node_name, vector<string> topics, protobus_cb cb)
{
    protobus_handle_t *handle = protobus_init(node_name);
    if (handle != nullptr)
    {
        for (auto it = topics.begin(); it != topics.end(); it++)
        {

            protobus_add_subscriber(handle, (*it).data(), cb);
        }
    }
    return handle;
}
void protobus_cleanup(protobus_handle_t *handle)
{
    delete handle;
}
void protobus_send(protobus_handle_t *handle, const MSG::WrapperMessage &msg)
{

    std::unique_lock<std::mutex> lock(handle->queueMutex);
    // Wait until the queue size is below the threshold
    handle->queueCond.wait(lock, [&handle]
                           { return handle->msgQueue.size() <= 1000; });
    handle->msgQueue.push(std::make_shared<MSG::WrapperMessage>(msg));
    handle->queueCond.notify_one();
    // lock.unlock();
}
void protobus_add_subscriber(protobus_handle_t *handle, const char *topic, protobus_cb cb)
{
    std::lock_guard<std::mutex> lock(handle->topicMutex);
#ifdef TOPIC_MAP
    handle->topicMap[topic] = cb;
    handle->sub->set(zmq::sockopt::subscribe, topic);
    handle->topicCond.notify_one();
#else
    string topicStr(topic);
    std::vector<std::pair<std::string, protobus_cb>>::iterator it;
    for (it = handle->topicVec.begin(); it != handle->topicVec.end(); it++)
    {
        if ((*it).first == topicStr)
        {
            break;
        }
    }
    if (it == handle->topicVec.end())
    {
        handle->topicVec.push_back(std::make_pair(topicStr, cb));
        handle->sub->set(zmq::sockopt::subscribe, topic);
        handle->topicCond.notify_one();
    }
    else
    {
        std::cerr << "Topic already exists." << std::endl;
    }

#endif
}
void protobus_del_subscriber(protobus_handle_t *handle, const char *topic)
{
    std::lock_guard<std::mutex> lock(handle->topicMutex);
#ifdef TOPIC_MAP
    auto it = handle->topicMap.find(topic);
    if (it != handle->topicMap.end())
    {
        handle->topicMap.erase(it);
        std::cout << "topic " << topic << "' removed from map." << std::endl;
    }
    else
    {
        std::cout << "topic " << topic << "' not found in map." << std::endl;
    }
#else
    std::vector<std::pair<std::string, protobus_cb>>::iterator it;
    for (it = handle->topicVec.begin(); it != handle->topicVec.end(); ++it)
    {
        if (it->first == topic)
        {
            break;
        }
    }
    if (it != handle->topicVec.end())
    {
        handle->topicVec.erase(it);
        std::cout << "topic '" << topic << "' removed from vector." << std::endl;
    }
    else
    {
        std::cout << "topic '" << topic << "' not found in vector." << std::endl;
    }
#endif
    handle->sub->set(zmq::sockopt::unsubscribe, topic);
}