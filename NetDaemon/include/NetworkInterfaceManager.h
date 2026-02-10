#ifndef NETWORK_INTERFACE_MANAGER_H
#define NETWORK_INTERFACE_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <functional>

/**
 * @brief 网络接口状态枚举
 */
enum class InterfaceState {
    UNKNOWN,
    DOWN,
    UP,
    TESTING
};

/**
 * @brief 网络接口类型枚举
 */
enum class InterfaceType {
    WIFI,
    MOBILE_DATA,
    ETHERNET,
    LOOPBACK,
    OTHER
};

/**
 * @brief 网络接口信息结构体
 */
struct NetworkInterface {
    std::string name;
    std::string ip_address;
    std::string netmask;
    std::string mac_address;
    InterfaceState state;
    InterfaceType type;
    int metric;
};

/**
 * @brief 网络接口管理器
 * 
 * 负责管理和配置网络接口，包括接口的启用/禁用、状态监控等
 */
class NetworkInterfaceManager {
public:
    using InterfaceCallback = std::function<void(const NetworkInterface&)>;

    NetworkInterfaceManager();
    ~NetworkInterfaceManager();

    /**
     * @brief 初始化网络接口管理器
     * @return 成功返回true，失败返回false
     */
    bool initialize();

    /**
     * @brief 获取所有网络接口
     * @return 网络接口列表
     */
    std::vector<NetworkInterface> getInterfaces() const;

    /**
     * @brief 根据名称获取网络接口
     * @param name 接口名称
     * @return 网络接口信息，如果不存在返回nullptr
     */
    NetworkInterface* getInterface(const std::string& name);

    /**
     * @brief 启用网络接口
     * @param name 接口名称
     * @return 成功返回true，失败返回false
     */
    bool bringUp(const std::string& name);

    /**
     * @brief 禁用网络接口
     * @param name 接口名称
     * @return 成功返回true，失败返回false
     */
    bool bringDown(const std::string& name);

    /**
     * @brief 配置网络接口IP地址
     * @param name 接口名称
     * @param ip_address IP地址
     * @param netmask 子网掩码
     * @return 成功返回true，失败返回false
     */
    bool setIPAddress(const std::string& name, const std::string& ip_address, const std::string& netmask);

    /**
     * @brief 设置接口metric值（用于路由优先级）
     * @param name 接口名称
     * @param metric metric值，值越小优先级越高
     * @return 成功返回true，失败返回false
     */
    bool setMetric(const std::string& name, int metric);

    /**
     * @brief 刷新接口列表
     * @return 成功返回true，失败返回false
     */
    bool refreshInterfaces();

    /**
     * @brief 注册接口状态变化回调
     * @param callback 回调函数
     */
    void registerCallback(InterfaceCallback callback);

private:
    std::map<std::string, NetworkInterface> interfaces_;
    InterfaceCallback state_callback_;
    bool initialized_;

    /**
     * @brief 读取网络接口信息（模拟实现）
     */
    void readInterfaceInfo();

    /**
     * @brief 执行网络命令（模拟实现）
     */
    bool executeCommand(const std::string& command);
};

#endif // NETWORK_INTERFACE_MANAGER_H