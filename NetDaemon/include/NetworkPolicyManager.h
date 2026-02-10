#ifndef NETWORK_POLICY_MANAGER_H
#define NETWORK_POLICY_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <functional>

/**
 * @brief 网络优先级策略类型
 */
enum class NetworkPriority {
    DEFAULT,
    HIGH,
    LOW,
    NEVER
};

/**
 * @brief 网络使用策略类型
 */
enum class NetworkUsagePolicy {
    ALLOWED,
    RESTRICTED,
    BLOCKED
};

/**
 * @brief 网络策略结构体
 */
struct NetworkPolicy {
    std::string id;
    std::string network_id;
    NetworkPriority priority;
    NetworkUsagePolicy usage_policy;
    bool metered;  // 是否计量网络
    int bandwidth_limit;  // 带宽限制 (kbps)
    bool enabled;
};

/**
 * @brief 应用网络策略结构体
 */
struct AppNetworkPolicy {
    std::string package_name;
    std::string network_id;
    NetworkUsagePolicy policy;
    bool enabled;
};

/**
 * @brief 网络策略管理器
 * 
 * 负责根据ConnectivityService的策略执行网络配置
 * 实现网络优先级规则和处理网络切换逻辑
 */
class NetworkPolicyManager {
public:
    using PolicyCallback = std::function<void(const std::string&, bool)>;

    NetworkPolicyManager();
    ~NetworkPolicyManager();

    /**
     * @brief 初始化网络策略管理器
     * @return 成功返回true，失败返回false
     */
    bool initialize();

    /**
     * @brief 获取所有网络策略
     * @return 网络策略列表
     */
    std::vector<NetworkPolicy> getNetworkPolicies() const;

    /**
     * @brief 添加网络策略
     * @param policy 网络策略
     * @return 成功返回true，失败返回false
     */
    bool addNetworkPolicy(const NetworkPolicy& policy);

    /**
     * @brief 删除网络策略
     * @param policy_id 策略ID
     * @return 成功返回true，失败返回false
     */
    bool removeNetworkPolicy(const std::string& policy_id);

    /**
     * @brief 启用网络策略
     * @param policy_id 策略ID
     * @return 成功返回true，失败返回false
     */
    bool enableNetworkPolicy(const std::string& policy_id);

    /**
     * @brief 禁用网络策略
     * @param policy_id 策略ID
     * @return 成功返回true，失败返回false
     */
    bool disableNetworkPolicy(const std::string& policy_id);

    /**
     * @brief 设置网络优先级
     * @param network_id 网络ID
     * @param priority 优先级
     * @return 成功返回true，失败返回false
     */
    bool setNetworkPriority(const std::string& network_id, NetworkPriority priority);

    /**
     * @brief 获取网络优先级
     * @param network_id 网络ID
     * @return 网络优先级
     */
    NetworkPriority getNetworkPriority(const std::string& network_id);

    /**
     * @brief 设置网络使用策略
     * @param network_id 网络ID
     * @param policy 使用策略
     * @return 成功返回true，失败返回false
     */
    bool setNetworkUsagePolicy(const std::string& network_id, NetworkUsagePolicy policy);

    /**
     * @brief 获取应用网络策略
     * @return 应用网络策略列表
     */
    std::vector<AppNetworkPolicy> getAppNetworkPolicies() const;

    /**
     * @brief 添加应用网络策略
     * @param policy 应用网络策略
     * @return 成功返回true，失败返回false
     */
    bool addAppNetworkPolicy(const AppNetworkPolicy& policy);

    /**
     * @brief 删除应用网络策略
     * @param package_name 包名
     * @param network_id 网络ID
     * @return 成功返回true，失败返回false
     */
    bool removeAppNetworkPolicy(const std::string& package_name, const std::string& network_id);

    /**
     * @brief 设置网络计量属性
     * @param network_id 网络ID
     * @param metered 是否计量
     * @return 成功返回true，失败返回false
     */
    bool setNetworkMetered(const std::string& network_id, bool metered);

    /**
     * @brief 检查网络是否计量
     * @param network_id 网络ID
     * @return 是否计量
     */
    bool isNetworkMetered(const std::string& network_id);

    /**
     * @brief 应用WiFi优先策略
     * @param wifi_id WiFi网络ID
     * @param mobile_id 移动数据网络ID
     * @return 成功返回true，失败返回false
     */
    bool applyWifiPriorityPolicy(const std::string& wifi_id, const std::string& mobile_id);

    /**
     * @brief 处理网络切换
     * @param from_network 源网络ID
     * @param to_network 目标网络ID
     * @return 成功返回true，失败返回false
     */
    bool handleNetworkSwitch(const std::string& from_network, const std::string& to_network);

    /**
     * @brief 注册策略变化回调
     * @param callback 回调函数
     */
    void registerCallback(PolicyCallback callback);

    /**
     * @brief 获取当前活动网络ID
     * @return 当前活动网络ID
     */
    std::string getActiveNetwork() const;

    /**
     * @brief 设置当前活动网络
     * @param network_id 网络ID
     * @return 成功返回true，失败返回false
     */
    bool setActiveNetwork(const std::string& network_id);

private:
    std::map<std::string, NetworkPolicy> network_policies_;
    std::map<std::string, AppNetworkPolicy> app_policies_;
    PolicyCallback policy_callback_;
    bool initialized_;
    std::string active_network_;
    int next_policy_id_;

    /**
     * @brief 执行策略命令
     */
    bool executeCommand(const std::string& command);

    /**
     * @brief 应用网络策略到系统
     */
    bool applyNetworkPolicy(const NetworkPolicy& policy);

    /**
     * @brief 生成策略ID
     */
    std::string generatePolicyId();

    /**
     * @brief 获取优先级字符串
     */
    std::string getPriorityString(NetworkPriority priority);

    /**
     * @brief 获取使用策略字符串
     */
    std::string getUsagePolicyString(NetworkUsagePolicy policy);
};

#endif // NETWORK_POLICY_MANAGER_H