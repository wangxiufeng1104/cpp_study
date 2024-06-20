#include <iostream>
#include "zmq/zmq.hpp"
#define TCP_SUB "tcp://127.0.0.1:5555"
#define TCP_PUB "tcp://127.0.0.1:5556"
int main()
{
    std::cout << "Proxy Starting ..." << std::endl;

    zmq::context_t ctx;
    
    zmq::socket_t frontend(ctx, zmq::socket_type::xsub);
    frontend.bind(TCP_SUB);

    zmq::socket_t backend(ctx, zmq::socket_type::xpub);
    backend.bind(TCP_PUB);

    zmq::proxy(frontend, backend);

    frontend.close();
    
    backend.close();

    return 0;
}