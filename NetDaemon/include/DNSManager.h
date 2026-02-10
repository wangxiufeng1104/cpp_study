#ifndef DNS_MANAGER_H
#define DNS_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>

/**
 * @brief DNS服务器信息结构体
 */
struct DNSServer {
    std::string ip_address;
    int priority;
    std::string interface;  // 关联的网络接口
};

/**
 * @brief DNS查询类型枚举
 */
enum class DNSQueryType {
    A,      // IPv4地址
    AAAA,   // IPv6地址
    MX,     // 邮件交换
    CNAME,  // 别名
    TXT,    // 文本记录
    NS,     // 名称服务器
    PTR,    // 指针记录
    SRV     // 服务记录
};

/**
 * @brief DNS解析结果结构体
 */
struct DNSResolution {
    std::string hostname;
    std::vector<std::string> ip_addresses;
    std::string dns_server;
    int ttl;  // 生存时间（秒）
    bool success;
    std::string error_message;
};

/**
 * @brief DNS缓存条目
 */
struct DNSCacheEntry {
    std::string hostname;
    DNSResolution resolution;
    time_t timestamp;
    int ttl;
};

/**
 * @brief DNS管理器
 * 
 * 负责配置DNS服务器、管理DNS解析策略
 */
class DNSManager {
public:
    using DNSCallback = std::function<void(const std::string&, const DNSResolution&)>;

    DNSManager();
    ~DNSManager();

    /**
     * @brief 初始化DNS管理器
     * @return 成功返回true，失败返回false
     */
    bool initialize();

    /**
     * @brief 获取所有配置的DNS服务器
     * @return DNS服务器列表
     */
    std::vector<DNSServer> getDNSServers() const;

    /**
     * @brief 添加DNS服务器
     * @param server DNS服务器信息
     * @return 成功返回true，失败返回false
     */
    bool addDNSServer(const DNSServer& server);

    /**
     * @brief 删除DNS服务器
     * @param ip_address DNS服务器IP地址
     * @return 成功返回true，失败返回false
     */
    bool removeDNSServer(const std::string& ip_address);

    /**
     * @brief 设置默认DNS服务器
     * @param ip_address DNS服务器IP地址
     * @return 成功返回true，失败返回false
     */
    bool setDefaultDNSServer(const std::string& ip_address);

    /**
     * @brief 解析域名
     * @param hostname 主机名
     * @param query_type 查询类型
     * @return DNS解析结果
     */
    DNSResolution resolve(const std::string& hostname, DNSQueryType query_type = DNSQueryType::A);

    /**
     * @brief 反向DNS查询（IP到域名）
     * @param ip_address IP地址
     * @return DNS解析结果
     */
    DNSResolution reverseLookup(const std::string& ip_address);

    /**
     * @brief 清空DNS缓存
     * @return 成功返回true，失败返回false
     */
    bool clearCache();

    /**
     * @brief 刷新DNS缓存（更新过期条目）
     * @return 成功返回true，失败返回false
     */
    bool refreshCache();

    /**
     * @brief 获取DNS缓存信息
     * @return DNS缓存条目列表
     */
    std::vector<DNSCacheEntry> getCache() const;

    /**
     * @brief 设置DNS缓存TTL
     * @param ttl 缓存生存时间（秒）
     * @return 成功返回true，失败返回false
     */
    bool setCacheTTL(int ttl);

    /**
     * @brief 启用/禁用DNS缓存
     * @param enabled 是否启用
     * @return 成功返回true，失败返回false
     */
    bool setCacheEnabled(bool enabled);

    /**
     * @brief 配置系统DNS（写入/etc/resolv.conf）
     * @return 成功返回true，失败返回false
     */
    bool configureSystemDNS();

    /**
     * @brief 添加DNS搜索域
     * @param domain 搜索域
     * @return 成功返回true，失败返回false
     */
    bool addSearchDomain(const std::string& domain);

    /**
     * @brief 获取DNS搜索域列表
     * @return 搜索域列表
     */
    std::vector<std::string> getSearchDomains() const;

    /**
     * @brief 注册DNS解析回调
     * @param callback 回调函数
     */
    void registerCallback(DNSCallback callback);

    /**
     * @brief 获取DNS统计信息
     */
    struct DNSStats {
        unsigned long total_queries;
        unsigned long cache_hits;
        unsigned long cache_misses;
        unsigned long failed_queries;
    };
    DNSStats getStats() const;

private:
    std::map<std::string, DNSServer> dns_servers_;
    std::map<std::string, DNSCacheEntry> dns_cache_;
    std::vector<std::string> search_domains_;
    DNSCallback dns_callback_;
    bool initialized_;
    bool cache_enabled_;
    int cache_ttl_;
    DNSStats stats_;

    /**
     * @brief 执行DNS查询
     */
    DNSResolution performDNSQuery(const std::string& hostname, DNSQueryType query_type);

    /**
     * @brief 检查缓存中是否有结果
     */
    DNSCacheEntry* findInCache(const std::string& hostname);

    /**
     * @brief 添加到缓存
     */
    void addToCache(const std::string& hostname, const DNSResolution& resolution);

    /**
     * @brief 清理过期的缓存条目
     */
    void cleanupExpiredCache();

    /**
     * @brief 执行DNS命令
     */
    bool executeCommand(const std::string& command);

    /**
     * @brief 获取查询类型字符串
     */
    std::string getQueryTypeString(DNSQueryType type);
};

#endif // DNS_MANAGER_H