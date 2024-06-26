#ifndef __PROTOBUS_H
#define __PROTOBUS_H
#include "message.pb.h"
#include <vector>
#include <string>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "zmq/zmq.hpp"
#define TCP_SUB "tcp://127.0.0.1:5555"
#define TCP_PUB "tcp://127.0.0.1:5556"
typedef struct protobus_handle protobus_handle_t;
typedef void (*protobus_cb)(const MSG::WrapperMessage &msg);
protobus_handle_t *protobus_init(const char *node_name);
protobus_handle_t *protobus_init(const char *node_name, std::vector<std::string> topics, protobus_cb cb);
void protobus_cleanup(protobus_handle_t *handle);
void protobus_send(protobus_handle_t *handle, const MSG::WrapperMessage &msg);
void protobus_add_subscriber(protobus_handle_t *handle, const char *topic, protobus_cb cb);
void protobus_del_subscriber(protobus_handle_t *handle, const char *topic);
using namespace std;
//#define THREADSAFE_QUEUE
#ifdef THREADSAFE_QUEUE
template <typename T>
class threadsafe_queue
{
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
    std::atomic<bool> stop_flag = false;
public:
    threadsafe_queue() {}
    threadsafe_queue(threadsafe_queue const &other)
    {
        std::lock_guard<std::mutex> lk(other.mut);
        data_queue = other.data_queue;
    }
    void stop()
    {
        stop_flag = true;
    }
    void wait_push(const T &&new_value)
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]
                       { return (data_queue.size() <= 1000 || stop_flag); });
        data_queue.push(std::move(new_value));
        //lk.unlock();
        data_cond.notify_one();
    }
    void wait_and_pop(T &value)
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]
                       { return !data_queue.empty(); });
        value = data_queue.front();
        data_queue.pop();
    }
    T wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]
                       { return !data_queue.empty(); });
        T res = data_queue.front();
        data_queue.pop();
        lk.unlock();
        data_cond.notify_one();
        return res;
    }
    bool try_pop(T &value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return false;
        value = data_queue.front();
        data_queue.pop();
        return true;
    }
    T try_pop()
    {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return std::shared_ptr<T>();
        T res = data_queue.front();
        data_queue.pop();
        return res;
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};
#endif
class protobus
{
public:
    typedef void (*protobus_cb)(const MSG::WrapperMessage &msg);
    protobus(const char *node_name);
    protobus(const char *node_name, std::vector<std::string> topics, protobus_cb cb);
    ~protobus();
    void send(const MSG::WrapperMessage &msg);
    void add_subscriber(const char *topic, protobus_cb cb);
    void del_subscriber(const char *topic);

private:
    /* pub socket */
    zmq::socket_t *pub_sock = nullptr;
    /* sub ctx */
    zmq::context_t *pub_ctx = nullptr;
    /* sub socket */
    zmq::socket_t *sub_sock = nullptr;
    /* sub ctx */
    zmq::context_t *sub_ctx = nullptr;
    /* sender identify*/
    string identify;
    std::thread pub_task;
    std::thread sub_task;
    /* run status */
    std::atomic<bool> run_status = false;
    /* topic vector */
    std::mutex topic_mutex;
    std::condition_variable topic_cond;
    std::vector<std::pair<string, protobus_cb>> topic_vec;
    /* protobuf msg */
#ifndef THREADSAFE_QUEUE
    std::queue<std::shared_ptr<MSG::WrapperMessage>> msg_queue;
    std::mutex msg_mutex;
    std::condition_variable msg_cond;
#else
    threadsafe_queue<std::shared_ptr<MSG::WrapperMessage>> msg_queue;
#endif
    std::shared_ptr<MSG::WrapperMessage> get_msg();
    size_t send_msg(std::shared_ptr<MSG::WrapperMessage> msg);
    void pub_task_function();
    void sub_task_function();
};
#endif