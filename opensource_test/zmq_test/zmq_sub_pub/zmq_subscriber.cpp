#include <iostream>
#include "zmq/zmq.hpp"
#include "message.pb.h"
int main()
{
    std::cout << "start sub node ..." << std::endl;

    while(1)
    {
        zmq_sleep(1);
        std::cout << "running sub node ..." << std::endl;
    }
    return 0;
}