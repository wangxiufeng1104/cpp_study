// 输出格式请参考https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
#include <cstdio>
#include <chrono>
#include <iostream>
#define SPDLOG_NAME "spdlog"
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE // 必须定义这个宏,才能输出文件名和行号
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/fmt/bin_to_hex.h"
#include <signal.h>
#include <libgen.h>
#include "protobus.hpp"

using namespace std;
struct ubus_context *ctx;
uint32_t exit_flag = 1;
// 智能指针
std::shared_ptr<spdlog::logger> rotating_logger;

void signal_hander(int sig)
{
    printf("signal %d\n", sig);
    exit_flag = 0;
}

void protobus_callback(const MSG::WrapperMessage &msg)
{
    switch (msg.message_type_case())
    {
    case MSG::WrapperMessage::kLog:
    {
        std::string str = msg.log().log();
        rotating_logger->debug(str);
    }
    break;
    default:
    {
        std::cout << "unknow topic " << msg.topic() << ", msg.message_type_case() " << msg.message_type_case() << std::endl;
    }
    break;
    }
}
int main(int argc, char *argv[])
{
    // Create a file rotating logger with 5mb size max and 3 rotated files
    int ret = 0;
    signal(SIGTERM, signal_hander);
    signal(SIGINT, signal_hander);
    auto max_size = 1024 * 1024 * 100;
    auto max_files = 4;
    rotating_logger = spdlog::rotating_logger_mt(SPDLOG_NAME, "/tmp/log/log.txt", max_size, max_files);
    spdlog::flush_every(std::chrono::seconds(1));
    spdlog::set_default_logger(rotating_logger);
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("%v");
    std::shared_ptr bus = protobus::get_instance(basename(argv[0]));
    bus->add_subscriber("log", protobus_callback);

    while (exit_flag)
    {
        sleep(1);
    }
}
