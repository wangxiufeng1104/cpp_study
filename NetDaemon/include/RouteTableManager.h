#ifndef ROUTE_TABLE_MANAGER_H
#define ROUTE_TABLE_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <functional>

/**
 * @brief 路由类型枚举
 */
enum class RouteType {
    UNICAST,
    LOCAL,
    BROADCAST,
    MULTICAST,
    ANYCAST
};

/**
 * @brief 路由协议枚举
 */
enum class RouteProtocol {
    KERNEL,
    BOOT,
    STATIC,
    ICMP_REDIRECT,
    RA
};

/**
 * @brief 路由条目结构体
 */
struct RouteEntry {
    std::string destination;
    std::string gateway;
    std::string interface;
    int metric;
    RouteType type;
    RouteProtocol protocol;
    unsigned int table_id;
};

/**
 * @brief 路由表管理器
 * 
 * 负责维护和操作系统路由表，实现多网络环境下的路由选择
 * 特别支持WiFi优先上网的路由策略
 */
class RouteTableManager {
public:
    using RouteCallback = std::function<void(const RouteEntry&, bool)>;

    RouteTableManager();
    ~RouteTableManager();

    /**
     * @brief 初始化路由表管理器
     * @return 成功返回true，失败返回false
     */
    bool initialize();

    /**
     * @brief 获取所有路由条目
     * @return 路由条目列表
     */
    std::vector<RouteEntry> getRoutes() const;

    /**
     * @brief 获取默认路由
     * @return 默认路由条目，如果不存在返回nullptr
     */
    RouteEntry* getDefaultRoute();

    /**
     * @brief 添加路由条目
     * @param route 路由条目
     * @return 成功返回true，失败返回false
     */
    bool addRoute(const RouteEntry& route);

    /**
     * @brief 删除路由条目
     * @param destination 目标地址
     * @param gateway 网关地址
     * @return 成功返回true，失败返回false
     */
    bool deleteRoute(const std::string& destination, const std::string& gateway);

    /**
     * @brief 设置默认路由（实现WiFi优先）
     * @param interface 接口名称
     * @param gateway 网关地址
     * @param metric metric值
     * @return 成功返回true，失败返回false
     */
    bool setDefaultRoute(const std::string& interface, const std::string& gateway, int metric);

    /**
     * @brief 刷新路由表
     * @return 成功返回true，失败返回false
     */
    bool refreshRoutes();

    /**
     * @brief 根据接口获取路由
     * @param interface 接口名称
     * @return 该接口的路由条目列表
     */
    std::vector<RouteEntry> getRoutesByInterface(const std::string& interface);

    /**
     * @brief 启用WiFi优先路由策略
     * @param wifi_interface WiFi接口名称
     * @param mobile_interface 移动数据接口名称
     * @return 成功返回true，失败返回false
     */
    bool enableWifiPriority(const std::string& wifi_interface, 
                            const std::string& mobile_interface);

    /**
     * @brief 禁用WiFi优先路由策略
     * @return 成功返回true，失败返回false
     */
    bool disableWifiPriority();

    /**
     * @brief 注册路由变化回调
     * @param callback 回调函数
     */
    void registerCallback(RouteCallback callback);

    /**
     * @brief 清空指定表的路由
     * @param table_id 表ID
     * @return 成功返回true，失败返回false
     */
    bool flushTable(unsigned int table_id);

private:
    std::vector<RouteEntry> routes_;
    RouteCallback route_callback_;
    bool initialized_;
    bool wifi_priority_enabled_;
    std::string wifi_interface_;
    std::string mobile_interface_;

    /**
     * @brief 读取路由表信息（模拟实现）
     */
    void readRouteTable();

    /**
     * @brief 执行路由命令（模拟实现）
     */
    bool executeCommand(const std::string& command);

    /**
     * @brief 更新路由优先级
     */
    bool updateRoutePriorities();

    /**
     * @brief 排序路由表
     */
    void sortRoutes();
};

#endif // ROUTE_TABLE_MANAGER_H