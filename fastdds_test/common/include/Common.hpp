/**
 * @file Common.hpp
 * @brief FastDDS测试项目公共头文件
 * 
 * 包含公共定义、工具函数和常用类型
 */

#ifndef FASTDDS_TEST_COMMON_HPP
#define FASTDDS_TEST_COMMON_HPP

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <signal.h>

namespace fastdds_test {

// 全局退出标志
extern std::atomic<bool> g_shutdown;

/**
 * @brief 初始化信号处理
 * @param sig 信号编号
 */
inline void signal_handler(int sig) {
    std::cout << "\n接收到信号 " << sig << "，正在退出..." << std::endl;
    g_shutdown = true;
}

/**
 * @brief 设置信号处理
 */
inline void setup_signal_handler() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
}

/**
 * @brief 等待用户输入
 */
inline void wait_for_input() {
    std::cout << "按Enter键退出..." << std::endl;
    std::cin.get();
}

/**
 * @brief 休眠指定毫秒数
 * @param ms 毫秒数
 */
inline void sleep_ms(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

/**
 * @brief 获取当前时间戳字符串
 * @return 时间戳字符串
 */
inline std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
    return std::string(buffer);
}

} // namespace fastdds_test

#endif // FASTDDS_TEST_COMMON_HPP