#include "NetworkPolicyManager.h"
#include <iostream>
#include <sstream>
#include <algorithm>

NetworkPolicyManager::NetworkPolicyManager()
    : initialized_(false), next_policy_id_(1) {
}

NetworkPolicyManager::~NetworkPolicyManager() {
}

bool NetworkPolicyManager::initialize() {
    if (initialized_) {
        return true;
    }

    // 添加默认的网络策略
    NetworkPolicy wifi_policy;
    wifi_policy.id = "policy_001";
    wifi_policy.network_id = "wifi_network";
    wifi_policy.priority = NetworkPriority::HIGH;
    wifi_policy.usage_policy = NetworkUsagePolicy::ALLOWED;
    wifi_policy.metered = false;
    wifi_policy.bandwidth_limit = 0;
    wifi_policy.enabled = true;
    network_policies_[wifi_policy.id] = wifi_policy;

    NetworkPolicy mobile_policy;
    mobile_policy.id = "policy_002";
    mobile_policy.network_id = "mobile_network";
    mobile_policy.priority = NetworkPriority::LOW;
    mobile_policy.usage_policy = NetworkUsagePolicy::ALLOWED;
    mobile_policy.metered = true;
    mobile_policy.bandwidth_limit = 1000;  // 1 Mbps
    mobile_policy.enabled = true;
    network_policies_[mobile_policy.id] = mobile_policy;

    initialized_ = true;
    return true;
}

std::vector<NetworkPolicy> NetworkPolicyManager::getNetworkPolicies() const {
    std::vector<NetworkPolicy> result;
    for (const auto& pair : network_policies_) {
        result.push_back(pair.second);
    }
    return result;
}

bool NetworkPolicyManager::addNetworkPolicy(const NetworkPolicy& policy) {
    std::string policy_id = policy.id.empty() ? generatePolicyId() : policy.id;
    
    NetworkPolicy new_policy = policy;
    new_policy.id = policy_id;
    
    network_policies_[policy_id] = new_policy;
    applyNetworkPolicy(new_policy);
    
    if (policy_callback_) {
        policy_callback_(policy_id, true);
    }

    std::cout << "[NetworkPolicyManager] Added network policy: " << policy_id 
              << " for network: " << policy.network_id << std::endl;
    return true;
}

bool NetworkPolicyManager::removeNetworkPolicy(const std::string& policy_id) {
    auto it = network_policies_.find(policy_id);
    if (it == network_policies_.end()) {
        return false;
    }

    network_policies_.erase(it);
    
    if (policy_callback_) {
        policy_callback_(policy_id, false);
    }

    std::cout << "[NetworkPolicyManager] Removed network policy: " << policy_id << std::endl;
    return true;
}

bool NetworkPolicyManager::enableNetworkPolicy(const std::string& policy_id) {
    auto it = network_policies_.find(policy_id);
    if (it == network_policies_.end()) {
        return false;
    }

    it->second.enabled = true;
    applyNetworkPolicy(it->second);
    return true;
}

bool NetworkPolicyManager::disableNetworkPolicy(const std::string& policy_id) {
    auto it = network_policies_.find(policy_id);
    if (it == network_policies_.end()) {
        return false;
    }

    it->second.enabled = false;
    return true;
}

bool NetworkPolicyManager::setNetworkPriority(const std::string& network_id, NetworkPriority priority) {
    for (auto& pair : network_policies_) {
        if (pair.second.network_id == network_id) {
            pair.second.priority = priority;
            applyNetworkPolicy(pair.second);
            std::cout << "[NetworkPolicyManager] Set network priority: " << network_id 
                      << " -> " << getPriorityString(priority) << std::endl;
            return true;
        }
    }
    return false;
}

NetworkPriority NetworkPolicyManager::getNetworkPriority(const std::string& network_id) {
    for (const auto& pair : network_policies_) {
        if (pair.second.network_id == network_id && pair.second.enabled) {
            return pair.second.priority;
        }
    }
    return NetworkPriority::DEFAULT;
}

bool NetworkPolicyManager::setNetworkUsagePolicy(const std::string& network_id, NetworkUsagePolicy policy) {
    for (auto& pair : network_policies_) {
        if (pair.second.network_id == network_id) {
            pair.second.usage_policy = policy;
            applyNetworkPolicy(pair.second);
            return true;
        }
    }
    return false;
}

std::vector<AppNetworkPolicy> NetworkPolicyManager::getAppNetworkPolicies() const {
    std::vector<AppNetworkPolicy> result;
    for (const auto& pair : app_policies_) {
        result.push_back(pair.second);
    }
    return result;
}

bool NetworkPolicyManager::addAppNetworkPolicy(const AppNetworkPolicy& policy) {
    std::string key = policy.package_name + ":" + policy.network_id;
    app_policies_[key] = policy;

    std::cout << "[NetworkPolicyManager] Added app network policy: " 
              << policy.package_name << " on " << policy.network_id << std::endl;
    return true;
}

bool NetworkPolicyManager::removeAppNetworkPolicy(const std::string& package_name, 
                                                    const std::string& network_id) {
    std::string key = package_name + ":" + network_id;
    auto it = app_policies_.find(key);
    if (it != app_policies_.end()) {
        app_policies_.erase(it);
        return true;
    }
    return false;
}

bool NetworkPolicyManager::setNetworkMetered(const std::string& network_id, bool metered) {
    for (auto& pair : network_policies_) {
        if (pair.second.network_id == network_id) {
            pair.second.metered = metered;
            std::cout << "[NetworkPolicyManager] Set network metered: " << network_id 
                      << " -> " << (metered ? "yes" : "no") << std::endl;
            return true;
        }
    }
    return false;
}

bool NetworkPolicyManager::isNetworkMetered(const std::string& network_id) {
    for (const auto& pair : network_policies_) {
        if (pair.second.network_id == network_id && pair.second.enabled) {
            return pair.second.metered;
        }
    }
    return false;
}

bool NetworkPolicyManager::applyWifiPriorityPolicy(const std::string& wifi_id, 
                                                    const std::string& mobile_id) {
    // 设置WiFi为高优先级
    setNetworkPriority(wifi_id, NetworkPriority::HIGH);
    
    // 设置移动数据为低优先级
    setNetworkPriority(mobile_id, NetworkPriority::LOW);
    
    // 设置移动数据为计量网络
    setNetworkMetered(mobile_id, true);
    setNetworkMetered(wifi_id, false);

    std::cout << "[NetworkPolicyManager] Applied WiFi priority policy" << std::endl;
    std::cout << "[NetworkPolicyManager] WiFi: " << wifi_id << " (HIGH priority, unmetered)" << std::endl;
    std::cout << "[NetworkPolicyManager] Mobile: " << mobile_id << " (LOW priority, metered)" << std::endl;

    return true;
}

bool NetworkPolicyManager::handleNetworkSwitch(const std::string& from_network, 
                                               const std::string& to_network) {
    std::cout << "[NetworkPolicyManager] Handling network switch: " 
              << from_network << " -> " << to_network << std::endl;

    // 检查目标网络策略
    for (const auto& pair : network_policies_) {
        if (pair.second.network_id == to_network && pair.second.enabled) {
            if (pair.second.usage_policy == NetworkUsagePolicy::BLOCKED) {
                std::cout << "[NetworkPolicyManager] Network " << to_network << " is blocked" << std::endl;
                return false;
            }
        }
    }

    // 更新活动网络
    active_network_ = to_network;

    std::cout << "[NetworkPolicyManager] Network switch completed successfully" << std::endl;
    return true;
}

void NetworkPolicyManager::registerCallback(PolicyCallback callback) {
    policy_callback_ = callback;
}

std::string NetworkPolicyManager::getActiveNetwork() const {
    return active_network_;
}

bool NetworkPolicyManager::setActiveNetwork(const std::string& network_id) {
    // 检查网络是否允许使用
    for (const auto& pair : network_policies_) {
        if (pair.second.network_id == network_id && pair.second.enabled) {
            if (pair.second.usage_policy == NetworkUsagePolicy::ALLOWED) {
                active_network_ = network_id;
                std::cout << "[NetworkPolicyManager] Active network set to: " << network_id << std::endl;
                return true;
            }
        }
    }
    return false;
}

bool NetworkPolicyManager::executeCommand(const std::string& command) {
    // 模拟执行命令
    std::cout << "[NetworkPolicyManager] Executing: " << command << std::endl;
    // 实际实现中会使用 system() 或 popen() 执行命令
    return true;
}

bool NetworkPolicyManager::applyNetworkPolicy(const NetworkPolicy& policy) {
    if (!policy.enabled) {
        return true;
    }

    // 根据策略应用规则
    if (policy.usage_policy == NetworkUsagePolicy::BLOCKED) {
        // 阻止该网络的使用
        std::ostringstream cmd;
        cmd << "iptables -A OUTPUT -o " << policy.network_id << " -j DROP";
        executeCommand(cmd.str());
    } else if (policy.bandwidth_limit > 0) {
        // 应用带宽限制
        std::ostringstream cmd;
        cmd << "tc qdisc add dev " << policy.network_id 
            << " root handle 1: htb default 10\n"
            << "tc class add dev " << policy.network_id 
            << " parent 1: classid 1:1 htb rate " << policy.bandwidth_limit << "kbps";
        executeCommand(cmd.str());
    }

    return true;
}

std::string NetworkPolicyManager::generatePolicyId() {
    return "policy_" + std::to_string(next_policy_id_++);
}

std::string NetworkPolicyManager::getPriorityString(NetworkPriority priority) {
    switch (priority) {
        case NetworkPriority::HIGH: return "HIGH";
        case NetworkPriority::LOW: return "LOW";
        case NetworkPriority::NEVER: return "NEVER";
        case NetworkPriority::DEFAULT: return "DEFAULT";
        default: return "DEFAULT";
    }
}

std::string NetworkPolicyManager::getUsagePolicyString(NetworkUsagePolicy policy) {
    switch (policy) {
        case NetworkUsagePolicy::ALLOWED: return "ALLOWED";
        case NetworkUsagePolicy::RESTRICTED: return "RESTRICTED";
        case NetworkUsagePolicy::BLOCKED: return "BLOCKED";
        default: return "ALLOWED";
    }
}