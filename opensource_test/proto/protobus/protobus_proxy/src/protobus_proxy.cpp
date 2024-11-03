#include <iostream>
#include "zmq/zmq.hpp"
#include "sys_utils.h"
#include <thread>
#include <signal.h>
#define TCP_SUB "tcp://127.0.0.1:5555"
#define TCP_PUB "tcp://127.0.0.1:5556"
zmq::context_t context(2);
#ifdef MONITOR_ENABLE
class MyMonitor : public zmq::monitor_t
{
public:
    MyMonitor(std::string name) : m_name(name) {}
    void on_monitor_started() override
    {
        std::cout << m_name << " monitor start" << std::endl;
    }
    void on_event_listening(const zmq_event_t &event_, const char *addr_)
    {
        std::cout << m_name << " monitor listening" << std::endl;
    }
    void on_event_accept_failed(const zmq_event_t &event_, const char *addr_)
    {
        std::cout << m_name << " monitor accept faile" << std::endl;
    }
    void on_event_handshake_succeeded(const zmq_event_t &event_, const char *addr_)
    {
        std::cout << m_name << " monitor handshake succeeded" << std::endl;
    }
    void on_event_accepted(const zmq_event_t &event_, const char *addr_)
    {
        std::cout << m_name << " monitor accept" << std::endl;
    }

    void on_event_disconnected(const zmq_event_t &event, const char *addr) override
    {
        std::cout << m_name << " Disconnected from " << addr << std::endl;
    }

    void on_event_closed(const zmq_event_t &event, const char *addr) override
    {
        std::cout << m_name << " Connection closed to " << addr << std::endl;
    }

private:
    std::string m_name;
};

void xsub_task(zmq::socket_t *socket)
{
    MyMonitor xsub_monitor("xsub_monitor");
    xsub_monitor.monitor(*socket, "inproc://xsub_monitor", ZMQ_EVENT_ALL);
}
void xpub_task(zmq::socket_t *socket)
{
    MyMonitor xpub_monitor("xpub_monitor");
    xpub_monitor.monitor(*socket, "inproc://xpub_monitor", ZMQ_EVENT_ALL);
}
#endif
void sig_handle(int sig_num)
{
    switch (sig_num)
    {
    case SIGTERM:
    case SIGINT:
    {
        context.shutdown();
    }
    break;
    default:
        break;
    }
}
int main()
{
    std::cout << "Proxy Starting ..." << std::endl;
    becomeSingle("protobus_proxy");

    signal(SIGTERM, sig_handle);
    signal(SIGINT, sig_handle);

    zmq::socket_t frontend(context, zmq::socket_type::xsub);

    frontend.bind(TCP_SUB);

    zmq::socket_t backend(context, zmq::socket_type::xpub);

    backend.bind(TCP_PUB);
#ifdef MONITOR_ENABLE
    std::thread tSub(xsub_task, &frontend);
    tSub.detach();
    std::thread tPub(xpub_task, &backend);
    tPub.detach();
#endif
    try
    {
        zmq::proxy(frontend, backend);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << "\n";
    }

    frontend.close();
    backend.close();

    std::cout << "GoodBye" << std::endl;

    return 0;
}