#include <iostream>
#include "protobus.h"
#include <libgen.h>

protobus_handle_t *handle = nullptr;
int main(int argc, char **argv)
{
    std::cout << "cpp node ...." << std::endl;
    handle = protobus_init(basename(argv[0]));

    std::cout << "name:" << basename(argv[0]) << std::endl;
    protobus_cleanup(handle);
    return 0;
}