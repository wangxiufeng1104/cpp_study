#include <iostream>
#include "protobus.hpp"
#include <libgen.h>
#include <unistd.h>
#include "google/protobuf/util/time_util.h"
#include <sstream>
#include <iomanip>
#include <random>
#include <string>
#include <thread>
#include <sys/time.h>
#include <signal.h>
using namespace std;
using namespace std::chrono_literals;
using google::protobuf::Timestamp;
using google::protobuf::util::TimeUtil;

MSG::WrapperMessage wrapper_msg;
MSG::msg_people people_msg;
MSG::msg_address addr_msg;
#define SEND_COUNT 2000000
int sleep_flag = 0;
int loop_count = 0;
uint64_t people_count = 0;
uint64_t address_count = 0;
void print_with_timestamp(const char *label, size_t count)
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm *local_time = std::localtime(&now_time);

    // Print timestamp and count
    printf("[%02d-%02d-%02d %02d:%02d:%02d] %s %lu\n",
           local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday,
           local_time->tm_hour, local_time->tm_min, local_time->tm_sec,
           label, count);
}

void sig_handle(int sig_num)
{
    switch (sig_num)
    {
    case SIGUSR1:
    {
        // sleep_flag = !sleep_flag;
        // if (sleep_flag)
        // {
        print_with_timestamp("people_count", people_count);
        print_with_timestamp("address_count", address_count);
        fflush(stdout);
        // }
    }
    break;
    default:
        break;
    }
}
int main(int argc, char **argv)
{
    // signal(SIGTERM, sig_handle);
    // signal(SIGINT, sig_handle);
    signal(SIGUSR1, sig_handle);
    std::shared_ptr bus = protobus::get_instance(basename(argv[0]));
    sleep(1);
    while (1)
    {
        if (sleep_flag == 0)
        {
            if (loop_count % 2 == 0)
            {
                addr_msg.set_city("abc");
                addr_msg.set_street("567");
                address_count = addr_msg.count() + 1;
                addr_msg.set_count(address_count);

                wrapper_msg.set_topic("address");
                *wrapper_msg.mutable_address() = addr_msg;
            }
            else
            {
                people_msg.set_name("arno");
                people_msg.set_age(20);
                people_count = people_msg.count() + 1;
                people_msg.set_count(people_count);

                wrapper_msg.set_topic("people");
                *wrapper_msg.mutable_people() = people_msg;
            }
            Timestamp timestamp;
            timestamp.set_seconds(time(NULL));
            timestamp.set_nanos(0);
            *wrapper_msg.mutable_timestamp() = timestamp;
            bus->send(wrapper_msg);
            loop_count++;
            // usleep(10);
        }
        else
        {
            sleep(1);
        }
    }
}