#include "zmq/zmq.hpp"
#include "protobus.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <queue>
#include "google/protobuf/util/time_util.h"
#include "message.pb.h"
#include <unordered_map>
using namespace std;
using google::protobuf::Timestamp;
using google::protobuf::util::TimeUtil;

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
    /* topic map */
    std::unordered_map<string, protobus_cb> topicMap;
    /* protobuf msg */
    std::queue<std::shared_ptr<MSG::WrapperMessage>> msgQueue;
    std::mutex queueMutex;
    std::condition_variable queueCond;
};
static void subscriber_init(protobus_handle_t *handle);
static size_t send_msg(protobus_handle_t *handle, std::shared_ptr<MSG::WrapperMessage> msg)
{
    std::unique_ptr<uint8_t[]> dataBuf;
    uint8_t *bufPtr;
    bool releaseFlag = false;
    size_t sendSize = msg->ByteSizeLong();
    std::cout << "send topic " << msg->topic() << endl;
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
        handle->pub->send(topic, zmq::send_flags::sndmore);
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

        handle->pub->send(zmq_msg, zmq::send_flags::dontwait);
        std::cout << "send msg" << std::endl;
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
                           {std::cout << "handle->msgQueue.size() = "<< handle->msgQueue.size() << std::endl; 
                            return !handle->msgQueue.empty(); });
    auto msgPtr = handle->msgQueue.front();
    std::cout << "queue size " << handle->msgQueue.size() << std::endl;
    handle->msgQueue.pop();
    lock.unlock();
    return msgPtr;
}
static void sub_task(protobus_handle_t *handle)
{
    
    printf("start sub task\n");
    while (handle->isRunning)
    {

        std::unique_lock<mutex> lock(handle->topicMutex);
        handle->queueCond.wait(lock, [&handle]
                               { return !handle->topicMap.empty(); });
        lock.unlock();

        zmq::message_t topic;
        zmq::message_t zmq_msg;

        zmq::recv_result_t result = handle->sub->recv(topic);
        if (result.has_value())
        {
            std::string topic(static_cast<char *>(topic.data()), topic.size());
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
    zmq::context_t ctx(1);
    handle->pub = new zmq::socket_t(ctx, zmq::socket_type::pub);
    handle->pub->connect(TCP_SUB);
    printf("start pub task\n");
    while (handle->isRunning)
    {
        auto msgPtr = get_msg(handle);
        if (msgPtr != nullptr)
        {
            std::cout << "get msg ,start send" << std::endl;
            sendSize = send_msg(handle, msgPtr);
            if (sendSize != msgPtr->ByteSizeLong())
            {
                std::cerr << "send msg failed ,ret %d" << sendSize << std::endl;
            }
        }
    }
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
        printf("%d subscriber_init \n", __LINE__);
    }
    catch (const std::exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        delete handle;
        handle = nullptr;
    }
    printf("%d return handle %p \n", __LINE__, handle);
    return handle;
}
protobus_handle_t *protobus_init(const char *node_name, vector<string> topics, protobus_cb cb)
{
    protobus_handle_t *handle = protobus_init(node_name);
    if (handle != nullptr)
    {
        for (auto it = topics.begin(); it != topics.end(); it++)
        {

            printf("add sub topic %s\n", (*it).data());
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
    std::lock_guard<std::mutex> lock(handle->queueMutex);
    handle->msgQueue.push(std::make_shared<MSG::WrapperMessage>(msg));
    std::cout << "push " << msg.topic() << std::endl;
    handle->queueCond.notify_one();
}
static void subscriber_init(protobus_handle_t *handle)
{
    handle->sub_ctx = new zmq::context_t(1);
    handle->sub = new zmq::socket_t(*handle->sub_ctx, zmq::socket_type::sub);
    handle->sub->connect(TCP_PUB);
    thread t(sub_task, handle);
    t.detach();
    printf("%d subscriber_init \n", __LINE__);
}
void protobus_add_subscriber(protobus_handle_t *handle, const char *topic, protobus_cb cb)
{
    printf("handle %p\n", handle);
    std::lock_guard<std::mutex> lock(handle->topicMutex);
    handle->topicMap[topic] = cb;
    handle->sub->set(zmq::sockopt::subscribe, topic);
    handle->topicCond.notify_one();
}
void protobus_del_subscriber(protobus_handle_t *handle, const char *topic)
{
    std::lock_guard<std::mutex> lock(handle->topicMutex);
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
    handle->sub->set(zmq::sockopt::unsubscribe, topic);
}