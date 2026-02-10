#include "FirewallManager.h"
#include <iostream>
#include <sstream>
#include <iomanip>

FirewallManager::FirewallManager()
    : initialized_(false), next_rule_id_(1) {
}

FirewallManager::~FirewallManager() {
}

bool FirewallManager::initialize() {
    if (initialized_) {
        return true;
    }

    // 添加一些默认的防火墙规则
    FirewallRule default_rule;
    default_rule.id = "rule_001";
    default_rule.table = IptablesTable::FILTER;
    default_rule.chain = IptablesChain::INPUT;
    default_rule.operation = IptablesOperation::APPEND;
    default_rule.protocol = "tcp";
    default_rule.action = "ACCEPT";
    default_rule.enabled = true;
    firewall_rules_[default_rule.id] = default_rule;

    initialized_ = true;
    return true;
}

std::vector<FirewallRule> FirewallManager::getFirewallRules() const {
    std::vector<FirewallRule> result;
    for (const auto& pair : firewall_rules_) {
        result.push_back(pair.second);
    }
    return result;
}

bool FirewallManager::addFirewallRule(const FirewallRule& rule) {
    std::string rule_id = rule.id.empty() ? ("rule_" + std::to_string(next_rule_id_++)) : rule.id;
    
    FirewallRule new_rule = rule;
    new_rule.id = rule_id;
    
    // 执行iptables命令
    std::string cmd = buildIptablesCommand(new_rule);
    if (executeCommand(cmd)) {
        firewall_rules_[rule_id] = new_rule;
        if (firewall_callback_) {
            firewall_callback_(rule_id, true);
        }
        return true;
    }

    return false;
}

bool FirewallManager::deleteFirewallRule(const std::string& rule_id) {
    auto it = firewall_rules_.find(rule_id);
    if (it == firewall_rules_.end()) {
        return false;
    }

    // 执行删除命令
    FirewallRule rule = it->second;
    rule.operation = IptablesOperation::DELETE;
    std::string cmd = buildIptablesCommand(rule);
    
    if (executeCommand(cmd)) {
        firewall_rules_.erase(it);
        if (firewall_callback_) {
            firewall_callback_(rule_id, false);
        }
        return true;
    }

    return false;
}

bool FirewallManager::enableFirewallRule(const std::string& rule_id) {
    auto it = firewall_rules_.find(rule_id);
    if (it == firewall_rules_.end()) {
        return false;
    }

    it->second.enabled = true;
    std::string cmd = buildIptablesCommand(it->second);
    return executeCommand(cmd);
}

bool FirewallManager::disableFirewallRule(const std::string& rule_id) {
    auto it = firewall_rules_.find(rule_id);
    if (it == firewall_rules_.end()) {
        return false;
    }

    // 删除规则来禁用它
    FirewallRule rule = it->second;
    rule.operation = IptablesOperation::DELETE;
    std::string cmd = buildIptablesCommand(rule);
    
    if (executeCommand(cmd)) {
        it->second.enabled = false;
        return true;
    }

    return false;
}

bool FirewallManager::flushChain(IptablesTable table, IptablesChain chain) {
    std::ostringstream cmd;
    cmd << "iptables -t " << getTableName(table) 
        << " -F " << getChainName(chain);
    
    return executeCommand(cmd.str());
}

bool FirewallManager::setDefaultPolicy(IptablesTable table, IptablesChain chain, 
                                        const std::string& policy) {
    std::ostringstream cmd;
    cmd << "iptables -t " << getTableName(table)
        << " -P " << getChainName(chain) << " " << policy;
    
    return executeCommand(cmd.str());
}

std::vector<TrafficControlRule> FirewallManager::getTrafficControlRules() const {
    std::vector<TrafficControlRule> result;
    for (const auto& pair : tc_rules_) {
        result.push_back(pair.second);
    }
    return result;
}

bool FirewallManager::addTrafficControlRule(const TrafficControlRule& rule) {
    std::string rule_id = rule.id.empty() ? ("tc_" + std::to_string(next_rule_id_++)) : rule.id;
    
    TrafficControlRule new_rule = rule;
    new_rule.id = rule_id;
    
    // 模拟执行tc命令
    std::ostringstream cmd;
    cmd << "tc qdisc add dev " << rule.interface
        << " root handle 1: htb default 10\n"
        << "tc class add dev " << rule.interface
        << " parent 1: classid 1:1 htb rate " << rule.rate << "kbps\n"
        << "tc qdisc add dev " << rule.interface
        << " parent 1:1 handle 10: netem delay " << rule.latency << "ms"
        << " loss " << rule.loss << "%";
    
    if (executeCommand(cmd.str())) {
        tc_rules_[rule_id] = new_rule;
        return true;
    }

    return false;
}

bool FirewallManager::deleteTrafficControlRule(const std::string& rule_id) {
    auto it = tc_rules_.find(rule_id);
    if (it == tc_rules_.end()) {
        return false;
    }

    // 清除接口的流量控制
    std::ostringstream cmd;
    cmd << "tc qdisc del dev " << it->second.interface << " root";
    
    if (executeCommand(cmd.str())) {
        tc_rules_.erase(it);
        return true;
    }

    return false;
}

bool FirewallManager::clearTrafficControl(const std::string& interface) {
    std::ostringstream cmd;
    cmd << "tc qdisc del dev " << interface << " root";
    
    // 删除所有使用该接口的规则
    auto it = tc_rules_.begin();
    while (it != tc_rules_.end()) {
        if (it->second.interface == interface) {
            it = tc_rules_.erase(it);
        } else {
            ++it;
        }
    }

    return executeCommand(cmd.str());
}

bool FirewallManager::enableNAT(const std::string& interface) {
    // 启用IP转发
    std::string sysctl_cmd = "sysctl -w net.ipv4.ip_forward=1";
    executeCommand(sysctl_cmd);

    // 添加NAT规则
    std::ostringstream nat_cmd;
    nat_cmd << "iptables -t nat -A POSTROUTING -o " << interface
            << " -j MASQUERADE";
    
    std::ostringstream forward_cmd;
    forward_cmd << "iptables -A FORWARD -i " << interface
                 << " -j ACCEPT\n"
                 << "iptables -A FORWARD -o " << interface
                 << " -j ACCEPT";

    return executeCommand(nat_cmd.str()) && 
           executeCommand(forward_cmd.str());
}

bool FirewallManager::disableNAT(const std::string& interface) {
    std::ostringstream nat_cmd;
    nat_cmd << "iptables -t nat -D POSTROUTING -o " << interface
            << " -j MASQUERADE";
    
    // 禁用IP转发
    std::string sysctl_cmd = "sysctl -w net.ipv4.ip_forward=0";
    
    executeCommand(sysctl_cmd);
    return executeCommand(nat_cmd.str());
}

bool FirewallManager::portForward(const std::string& external_ip, int external_port,
                                   const std::string& internal_ip, int internal_port,
                                   const std::string& protocol) {
    std::ostringstream cmd;
    cmd << "iptables -t nat -A PREROUTING -p " << protocol
        << " -d " << external_ip << " --dport " << external_port
        << " -j DNAT --to-destination " << internal_ip << ":" << internal_port << "\n"
        << "iptables -A FORWARD -p " << protocol
        << " -d " << internal_ip << " --dport " << internal_port
        << " -j ACCEPT";

    return executeCommand(cmd.str());
}

void FirewallManager::registerCallback(FirewallCallback callback) {
    firewall_callback_ = callback;
}

std::string FirewallManager::buildIptablesCommand(const FirewallRule& rule) {
    std::ostringstream cmd;
    cmd << "iptables -t " << getTableName(rule.table)
        << " " << getOperationName(rule.operation)
        << " " << getChainName(rule.chain);

    if (!rule.protocol.empty()) {
        cmd << " -p " << rule.protocol;
    }

    if (!rule.src_ip.empty()) {
        cmd << " -s " << rule.src_ip;
    }

    if (!rule.dst_ip.empty()) {
        cmd << " -d " << rule.dst_ip;
    }

    if (!rule.src_port.empty()) {
        cmd << " --sport " << rule.src_port;
    }

    if (!rule.dst_port.empty()) {
        cmd << " --dport " << rule.dst_port;
    }

    if (!rule.extra_params.empty()) {
        cmd << " " << rule.extra_params;
    }

    cmd << " -j " << rule.action;

    return cmd.str();
}

bool FirewallManager::executeCommand(const std::string& command) {
    // 模拟执行命令
    std::cout << "[FirewallManager] Executing: " << command << std::endl;
    // 实际实现中会使用 system() 或 popen() 执行命令
    return true;
}

std::string FirewallManager::getTableName(IptablesTable table) {
    switch (table) {
        case IptablesTable::FILTER: return "filter";
        case IptablesTable::NAT: return "nat";
        case IptablesTable::MANGLE: return "mangle";
        case IptablesTable::RAW: return "raw";
        default: return "filter";
    }
}

std::string FirewallManager::getChainName(IptablesChain chain) {
    switch (chain) {
        case IptablesChain::INPUT: return "INPUT";
        case IptablesChain::OUTPUT: return "OUTPUT";
        case IptablesChain::FORWARD: return "FORWARD";
        case IptablesChain::PREROUTING: return "PREROUTING";
        case IptablesChain::POSTROUTING: return "POSTROUTING";
        default: return "INPUT";
    }
}

std::string FirewallManager::getOperationName(IptablesOperation operation) {
    switch (operation) {
        case IptablesOperation::INSERT: return "-I";
        case IptablesOperation::APPEND: return "-A";
        case IptablesOperation::DELETE: return "-D";
        case IptablesOperation::REPLACE: return "-R";
        case IptablesOperation::FLUSH: return "-F";
        default: return "-A";
    }
}