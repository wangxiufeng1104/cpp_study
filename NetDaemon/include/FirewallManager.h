#ifndef FIREWALL_MANAGER_H
#define FIREWALL_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <functional>

/**
 * @brief iptables操作类型
 */
enum class IptablesOperation {
    INSERT,
    APPEND,
    DELETE,
    REPLACE,
    FLUSH
};

/**
 * @brief iptables表类型
 */
enum class IptablesTable {
    FILTER,
    NAT,
    MANGLE,
    RAW
};

/**
 * @brief iptables链类型
 */
enum class IptablesChain {
    INPUT,
    OUTPUT,
    FORWARD,
    PREROUTING,
    POSTROUTING
};

/**
 * @brief 防火墙规则结构体
 */
struct FirewallRule {
    std::string id;
    IptablesTable table;
    IptablesChain chain;
    IptablesOperation operation;
    std::string src_ip;
    std::string dst_ip;
    std::string src_port;
    std::string dst_port;
    std::string protocol;
    std::string action;  // ACCEPT, DROP, REJECT, etc.
    std::string extra_params;
    bool enabled;
};

/**
 * @brief 流量控制规则结构体
 */
struct TrafficControlRule {
    std::string id;
    std::string interface;
    unsigned int rate;       // 速率限制 (kbps)
    unsigned int burst;      // 突发流量
    unsigned int latency;    // 延迟 (ms)
    unsigned int loss;       // 丢包率 (百分比)
    bool enabled;
};

/**
 * @brief 防火墙和流量控制管理器
 * 
 * 负责管理iptables规则和流量控制，实现数据包过滤和转发
 */
class FirewallManager {
public:
    using FirewallCallback = std::function<void(const std::string&, bool)>;

    FirewallManager();
    ~FirewallManager();

    /**
     * @brief 初始化防火墙管理器
     * @return 成功返回true，失败返回false
     */
    bool initialize();

    /**
     * @brief 获取所有防火墙规则
     * @return 防火墙规则列表
     */
    std::vector<FirewallRule> getFirewallRules() const;

    /**
     * @brief 添加防火墙规则
     * @param rule 防火墙规则
     * @return 成功返回true，失败返回false
     */
    bool addFirewallRule(const FirewallRule& rule);

    /**
     * @brief 删除防火墙规则
     * @param rule_id 规则ID
     * @return 成功返回true，失败返回false
     */
    bool deleteFirewallRule(const std::string& rule_id);

    /**
     * @brief 启用防火墙规则
     * @param rule_id 规则ID
     * @return 成功返回true，失败返回false
     */
    bool enableFirewallRule(const std::string& rule_id);

    /**
     * @brief 禁用防火墙规则
     * @param rule_id 规则ID
     * @return 成功返回true，失败返回false
     */
    bool disableFirewallRule(const std::string& rule_id);

    /**
     * @brief 清空指定链的规则
     * @param table 表类型
     * @param chain 链类型
     * @return 成功返回true，失败返回false
     */
    bool flushChain(IptablesTable table, IptablesChain chain);

    /**
     * @brief 设置默认策略
     * @param table 表类型
     * @param chain 链类型
     * @param policy 策略（ACCEPT, DROP, REJECT）
     * @return 成功返回true，失败返回false
     */
    bool setDefaultPolicy(IptablesTable table, IptablesChain chain, const std::string& policy);

    /**
     * @brief 获取所有流量控制规则
     * @return 流量控制规则列表
     */
    std::vector<TrafficControlRule> getTrafficControlRules() const;

    /**
     * @brief 添加流量控制规则
     * @param rule 流量控制规则
     * @return 成功返回true，失败返回false
     */
    bool addTrafficControlRule(const TrafficControlRule& rule);

    /**
     * @brief 删除流量控制规则
     * @param rule_id 规则ID
     * @return 成功返回true，失败返回false
     */
    bool deleteTrafficControlRule(const std::string& rule_id);

    /**
     * @brief 清除接口的流量控制
     * @param interface 接口名称
     * @return 成功返回true，失败返回false
     */
    bool clearTrafficControl(const std::string& interface);

    /**
     * @brief 应用网络地址转换(NAT)规则
     * @param interface 外部接口
     * @return 成功返回true，失败返回false
     */
    bool enableNAT(const std::string& interface);

    /**
     * @brief 移除NAT规则
     * @param interface 外部接口
     * @return 成功返回true，失败返回false
     */
    bool disableNAT(const std::string& interface);

    /**
     * @brief 端口转发
     * @param external_ip 外部IP
     * @param external_port 外部端口
     * @param internal_ip 内部IP
     * @param internal_port 内部端口
     * @param protocol 协议（tcp/udp）
     * @return 成功返回true，失败返回false
     */
    bool portForward(const std::string& external_ip, int external_port,
                     const std::string& internal_ip, int internal_port,
                     const std::string& protocol);

    /**
     * @brief 注册防火墙规则变化回调
     * @param callback 回调函数
     */
    void registerCallback(FirewallCallback callback);

private:
    std::map<std::string, FirewallRule> firewall_rules_;
    std::map<std::string, TrafficControlRule> tc_rules_;
    FirewallCallback firewall_callback_;
    bool initialized_;
    int next_rule_id_;

    /**
     * @brief 生成iptables命令
     */
    std::string buildIptablesCommand(const FirewallRule& rule);

    /**
     * @brief 执行防火墙命令
     */
    bool executeCommand(const std::string& command);

    /**
     * @brief 获取表名称字符串
     */
    std::string getTableName(IptablesTable table);

    /**
     * @brief 获取链名称字符串
     */
    std::string getChainName(IptablesChain chain);

    /**
     * @brief 获取操作名称字符串
     */
    std::string getOperationName(IptablesOperation operation);
};

#endif // FIREWALL_MANAGER_H