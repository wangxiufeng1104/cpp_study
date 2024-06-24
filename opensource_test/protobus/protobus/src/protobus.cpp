#include "zmq/zmq.hpp"
#include <vector>
#include "protobus.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include "message.pb.h"
#include "google/protobuf/util/time_util.h"
using namespace std;

const char *TCP_SUB = "tcp://127.0.0.1:5555";
const char *TCP_PUB = "tcp://127.0.0.1:5556";
struct protobus_handle
{
    /* psb socket */
    zmq::socket_t *pub;
    /* sub socket */
    zmq::socket_t *sub;
    /* topic vec */
    vector<string> topic;
    /* sender identify*/
    string identify;
    /* run status */
    bool isRunning = false;

    std::mutex mutex;

    std::condition_variable cond;

    std::vector<MSG::WrapperMessage> msgVec;
};
void pub_task(protobus_handle_t *h)
{
    zmq::context_t ctx(1);
    h->pub = new zmq::socket_t(ctx, zmq::socket_type::pub);
    h->pub->connect(TCP_PUB);

    if (h->isRunning)
    {
        std::unique_lock<mutex> lock(h->mutex);
        h->cond.wait(lock, [&h]
                     { return !h->msgVec.empty(); });
        lock.unlock();
    }
}
protobus_handle_t *protobus_init(const char *node_name)
{
    protobus_handle_t *h = nullptr;
    try
    {
        h = new protobus_handle_t();

        h->pub = nullptr;
        h->sub = nullptr;

        if (node_name != nullptr)
        {
            h->identify = string(node_name);
        }
        h->isRunning = true;
        thread t(pub_task, h);
        t.detach();
    }
    catch (const std::exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        delete h;
        h = nullptr;
    }
    return h;
}
void protobus_cleanup(protobus_handle_t *handle)
{
    delete handle;
}
