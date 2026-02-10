#ifndef NETWORK_MONITOR_H
#define NETWORK_MONITOR_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>

/**
 * @brief 网络状态枚举
 */
enum class NetworkState {
    UNKNOWN,
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    SUSPENDED
};

/**
 * @brief 网络类型枚举
 */
enum class NetworkType {
    WIFI,
    MOBILE,
    ETHERNET,
    BLUETOOTH,
    VPN
};

/**
 * @brief 网络统计信息结构体
 */
struct NetworkStats {
    std::string interface;
    unsigned long rx_bytes;      // 接收字节数
    unsigned long tx_bytes;      // 发送字节数
    unsigned long rx_packets;    // 接收包数
    unsigned long tx_packets;    // 发送包数
    unsigned long rx_errors;     // 接收错误
    unsigned long tx_errors;     // 发送错误
    unsigned long rx_dropped;    // 丢弃的接收包
    unsigned long tx_dropped;    // 丢弃的发送包
};

/**
 * @brief 网络连接信息结构体
 */
struct NetworkConnection {
    std::string network_id;
    std::string interface;
    NetworkType type;
    NetworkState state;
    int signal_strength;         // 信号强度（0-100）
    std::string ip_address;
    std::string gateway;
    int link_speed;              // 链路速度 (Mbps)
    time_t connected_time;       // 连接时间戳
};

/**
 * @brief 网络事件类型
 */
enum class NetworkEventType {
    STATE_CHANGED,
    CONNECTION_ESTABLISHED,
    CONNECTION_LOST,
    SIGNAL_CHANGED,
    IP_CHANGED,
    ROUTE_CHANGED
};

/**
 * @brief 网络事件结构体
 */
struct NetworkEvent {
    NetworkEventType type;
    std::string network_id;
    std::string old_state;
    std::string new_state;
    time_t timestamp;
};

/**
 * @brief 网络状态监控器
 * 
 * 实时监控网络连接状态，检测网络变化，通知上层网络状态变化
 */
class NetworkMonitor {
public:
    using StateChangeCallback = std::function<void(const NetworkEvent&)>;
    using ConnectionCallback = std::function<void(const std::string&, bool)>;

    NetworkMonitor();
    ~NetworkMonitor();

    /**
     * @brief 初始化网络监控器
     * @return 成功返回true，失败返回false
     */
    bool initialize();

    /**
     * @brief 启动监控
     * @return 成功返回true，失败返回false
     */
    bool start();

    /**
     * @brief 停止监控
     * @return 成功返回true，失败返回false
     */
    bool stop();

    /**
     * @brief 获取所有网络连接
     * @return 网络连接列表
     */
    std::vector<NetworkConnection> getConnections() const;

    /**
     * @brief 根据网络ID获取连接信息
     * @param network_id 网络ID
     * @return 网络连接信息
     */
    NetworkConnection* getConnection(const std::string& network_id);

    /**
     * @brief 根据接口获取连接信息
     * @param interface 接口名称
     * @return 网络连接信息
     */
    NetworkConnection* getConnectionByInterface(const std::string& interface);

    /**
     * @brief 获取网络统计信息
     * @param interface 接口名称
     * @return 网络统计信息
     */
    NetworkStats getNetworkStats(const std::string& interface);

    /**
     * @brief 刷新网络状态
     * @return 成功返回true，失败返回false
     */
    bool refreshStatus();

    /**
     * @brief 检查网络连通性
     * @param host 主机地址
     * @return 成功返回true，失败返回false
     */
    bool checkConnectivity(const std::string& host = "8.8.8.8");

    /**
     * @brief 获取信号强度
     * @param interface 接口名称
     * @return 信号强度（0-100），-1表示不支持
     */
    int getSignalStrength(const std::string& interface);

    /**
     * @brief 获取链路速度
     * @param interface 接口名称
     * @return 链路速度（Mbps）
     */
    int getLinkSpeed(const std::string& interface);

    /**
     * @brief 设置监控间隔
     * @param interval 间隔时间（秒）
     * @return 成功返回true，失败返回false
     */
    bool setMonitoringInterval(int interval);

    /**
     * @brief 注册状态变化回调
     * @param callback 回调函数
     */
    void registerStateChangeCallback(StateChangeCallback callback);

    /**
     * @brief 注册连接状态回调
     * @param callback 回调函数
     */
    void registerConnectionCallback(ConnectionCallback callback);

    /**
     * @brief 获取活动连接
     * @return 活动连接信息
     */
    NetworkConnection* getActiveConnection();

    /**
     * @brief 模拟网络状态变化（用于测试）
     * @param network_id 网络ID
     * @param new_state 新状态
     * @return 成功返回true，失败返回false
     */
    bool simulateStateChange(const std::string& network_id, NetworkState new_state);

    /**
     * @brief 获取监控器状态
     */
    struct MonitorStats {
        bool running;
        int monitoring_interval;
        unsigned long total_events;
        unsigned long connection_changes;
    };
    MonitorStats getMonitorStats() const;

private:
    std::map<std::string, NetworkConnection> connections_;
    std::map<std::string, NetworkStats> stats_;
    StateChangeCallback state_callback_;
    ConnectionCallback connection_callback_;
    bool initialized_;
    bool running_;
    int monitoring_interval_;
    MonitorStats monitor_stats_;

    /**
     * @brief 读取网络状态
     */
    void readNetworkStatus();

    /**
     * @brief 读取网络统计信息
     */
    void readNetworkStats();

    /**
     * @brief 检测网络状态变化
     */
    void detectStateChanges();

    /**
     * @brief 触发网络事件
     */
    void triggerEvent(const NetworkEvent& event);

    /**
     * @brief 获取状态字符串
     */
    std::string getStateString(NetworkState state);

    /**
     * @brief 获取网络类型字符串
     */
    std::string getNetworkTypeString(NetworkType type);

    /**
     * @brief 执行监控命令
     */
    bool executeCommand(const std::string& command);
};

#endif // NETWORK_MONITOR_H