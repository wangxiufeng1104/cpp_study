/**
 * @file Common.cpp
 * @brief FastDDS测试项目公共源文件
 */

#include "Common.hpp"

namespace fastdds_test {

// 定义全局退出标志
std::atomic<bool> g_shutdown(false);

} // namespace fastdds_test