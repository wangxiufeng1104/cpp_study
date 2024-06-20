#include <iostream>
#include "zmq/zmq.hpp"
#define TCP_SUB "tcp://127.0.0.1:5555"
#define TCP_PUB "tcp://127.0.0.1:5556"

int main()
{
    std::cout << "Proxy Starting ..." << std::endl;

    zmq::context_t context(1);
    int io_threads = 4;
    context.set(zmq::ctxopt::io_threads, io_threads);

    zmq::socket_t frontend(context, zmq::socket_type::xsub);
    frontend.bind(TCP_SUB);

    zmq::socket_t backend(context, zmq::socket_type::xpub);
    backend.bind(TCP_PUB);

    zmq::proxy(frontend, backend);

    frontend.close();

    backend.close();

    return 0;
}