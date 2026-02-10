#include "NetworkInterfaceManager.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>

NetworkInterfaceManager::NetworkInterfaceManager()
    : initialized_(false), nl_sock_(-1), nl_initialized_(false) {
}

NetworkInterfaceManager::~NetworkInterfaceManager() {
    cleanupNetlink();
}

bool NetworkInterfaceManager::initialize() {
    if (initialized_) {
        return true;
    }

    // 初始化 netlink socket
    if (!initNetlink()) {
        std::cerr << "Failed to initialize netlink socket" << std::endl;
        return false;
    }

    // 读取网络接口信息
    readInterfaceInfo();

    initialized_ = true;
    return true;
}

std::vector<NetworkInterface> NetworkInterfaceManager::getInterfaces() const {
    std::vector<NetworkInterface> result;
    for (const auto& pair : interfaces_) {
        result.push_back(pair.second);
    }
    return result;
}

NetworkInterface* NetworkInterfaceManager::getInterface(const std::string& name) {
    auto it = interfaces_.find(name);
    if (it != interfaces_.end()) {
        return &it->second;
    }
    return nullptr;
}

bool NetworkInterfaceManager::bringUp(const std::string& name) {
    auto* iface = getInterface(name);
    if (!iface) {
        std::cerr << "Interface " << name << " not found" << std::endl;
        return false;
    }

    int ifindex = getInterfaceIndex(name);
    if (ifindex < 0) {
        std::cerr << "Failed to get interface index for " << name << std::endl;
        return false;
    }

    // 使用 netlink 设置接口状态为 UP
    struct {
        struct nlmsghdr nh;
        struct ifinfomsg ifinfo;
        char buf[256];
    } req;

    memset(&req, 0, sizeof(req));
    
    req.nh.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    req.nh.nlmsg_type = RTM_NEWLINK;
    req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.nh.nlmsg_pid = getpid();

    req.ifinfo.ifi_family = AF_UNSPEC;
    req.ifinfo.ifi_index = ifindex;
    req.ifinfo.ifi_change = IFF_UP;
    req.ifinfo.ifi_flags = IFF_UP;

    if (sendNetlinkMessage(&req, req.nh.nlmsg_len)) {
        iface->state = InterfaceState::UP;
        if (state_callback_) {
            state_callback_(*iface);
        }
        return true;
    }

    return false;
}

bool NetworkInterfaceManager::bringDown(const std::string& name) {
    auto* iface = getInterface(name);
    if (!iface) {
        std::cerr << "Interface " << name << " not found" << std::endl;
        return false;
    }

    int ifindex = getInterfaceIndex(name);
    if (ifindex < 0) {
        std::cerr << "Failed to get interface index for " << name << std::endl;
        return false;
    }

    // 使用 netlink 设置接口状态为 DOWN
    struct {
        struct nlmsghdr nh;
        struct ifinfomsg ifinfo;
        char buf[256];
    } req;

    memset(&req, 0, sizeof(req));
    
    req.nh.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    req.nh.nlmsg_type = RTM_NEWLINK;
    req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.nh.nlmsg_pid = getpid();

    req.ifinfo.ifi_family = AF_UNSPEC;
    req.ifinfo.ifi_index = ifindex;
    req.ifinfo.ifi_change = IFF_UP;
    req.ifinfo.ifi_flags = 0;

    if (sendNetlinkMessage(&req, req.nh.nlmsg_len)) {
        iface->state = InterfaceState::DOWN;
        if (state_callback_) {
            state_callback_(*iface);
        }
        return true;
    }

    return false;
}

bool NetworkInterfaceManager::setIPAddress(const std::string& name, 
                                            const std::string& ip_address, 
                                            const std::string& netmask) {
    auto* iface = getInterface(name);
    if (!iface) {
        std::cerr << "Interface " << name << " not found" << std::endl;
        return false;
    }

    int ifindex = getInterfaceIndex(name);
    if (ifindex < 0) {
        std::cerr << "Failed to get interface index for " << name << std::endl;
        return false;
    }

    // 将子网掩码转换为前缀长度
    uint32_t mask;
    if (inet_pton(AF_INET, netmask.c_str(), &mask) != 1) {
        std::cerr << "Invalid netmask: " << netmask << std::endl;
        return false;
    }
    
    // 计算前缀长度（32位中1的个数）
    int prefixlen = 0;
    uint32_t mask_ntohl = ntohl(mask);
    while (mask_ntohl) {
        prefixlen += mask_ntohl & 1;
        mask_ntohl >>= 1;
    }

    // 构造 netlink 消息
    struct {
        struct nlmsghdr nh;
        struct ifaddrmsg ifa;
        char buf[512];
    } req;

    memset(&req, 0, sizeof(req));
    
    req.nh.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
    req.nh.nlmsg_type = RTM_NEWADDR;
    req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_REPLACE | NLM_F_ACK;
    req.nh.nlmsg_pid = getpid();

    req.ifa.ifa_family = AF_INET;
    req.ifa.ifa_prefixlen = prefixlen;
    req.ifa.ifa_index = ifindex;
    req.ifa.ifa_scope = RT_SCOPE_UNIVERSE;

    // 将 IP 地址转换为网络字节序
    struct in_addr addr;
    if (inet_pton(AF_INET, ip_address.c_str(), &addr) != 1) {
        std::cerr << "Invalid IP address: " << ip_address << std::endl;
        return false;
    }

    // 添加 IFA_LOCAL 属性（本地地址）
    struct rtattr* rta = (struct rtattr*)req.buf;
    rta->rta_type = IFA_LOCAL;
    rta->rta_len = RTA_LENGTH(sizeof(addr));
    memcpy(RTA_DATA(rta), &addr, sizeof(addr));
    req.nh.nlmsg_len = NLMSG_ALIGN(req.nh.nlmsg_len) + RTA_ALIGN(rta->rta_len);

    // 添加 IFA_ADDRESS 属性（对等地址，通常与 IFA_LOCAL 相同）
    rta = (struct rtattr*)((char*)rta + RTA_ALIGN(rta->rta_len));
    rta->rta_type = IFA_ADDRESS;
    rta->rta_len = RTA_LENGTH(sizeof(addr));
    memcpy(RTA_DATA(rta), &addr, sizeof(addr));
    req.nh.nlmsg_len = NLMSG_ALIGN(req.nh.nlmsg_len) + RTA_ALIGN(rta->rta_len);

    // 添加 IFA_LABEL 属性（接口名称）
    rta = (struct rtattr*)((char*)rta + RTA_ALIGN(rta->rta_len));
    rta->rta_type = IFA_LABEL;
    rta->rta_len = RTA_LENGTH(name.length() + 1);
    memcpy(RTA_DATA(rta), name.c_str(), name.length() + 1);
    req.nh.nlmsg_len = NLMSG_ALIGN(req.nh.nlmsg_len) + RTA_ALIGN(rta->rta_len);

    if (sendNetlinkMessage(&req, req.nh.nlmsg_len)) {
        iface->ip_address = ip_address;
        iface->netmask = netmask;
        if (state_callback_) {
            state_callback_(*iface);
        }
        return true;
    }

    return false;
}

bool NetworkInterfaceManager::setMetric(const std::string& name, int metric) {
    auto* iface = getInterface(name);
    if (!iface) {
        std::cerr << "Interface " << name << " not found" << std::endl;
        return false;
    }

    int ifindex = getInterfaceIndex(name);
    if (ifindex < 0) {
        std::cerr << "Failed to get interface index for " << name << std::endl;
        return false;
    }

    // 使用 netlink 设置接口 metric
    struct {
        struct nlmsghdr nh;
        struct ifinfomsg ifinfo;
        char buf[256];
    } req;

    memset(&req, 0, sizeof(req));
    
    req.nh.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    req.nh.nlmsg_type = RTM_NEWLINK;
    req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.nh.nlmsg_pid = getpid();

    req.ifinfo.ifi_family = AF_UNSPEC;
    req.ifinfo.ifi_index = ifindex;
    req.ifinfo.ifi_change = 0xffffffff;

    // 添加 IFLA_IFNAME 属性
    struct rtattr* rta = (struct rtattr*)req.buf;
    rta->rta_type = IFLA_IFNAME;
    rta->rta_len = RTA_LENGTH(name.length() + 1);
    memcpy(RTA_DATA(rta), name.c_str(), name.length() + 1);
    req.nh.nlmsg_len = NLMSG_ALIGN(req.nh.nlmsg_len) + RTA_ALIGN(rta->rta_len);

    // 添加 IFLA_PRIORITY 属性（metric）
    rta = (struct rtattr*)((char*)rta + RTA_ALIGN(rta->rta_len));
    rta->rta_type = IFLA_PRIORITY;
    rta->rta_len = RTA_LENGTH(sizeof(uint32_t));
    *(uint32_t*)RTA_DATA(rta) = metric;
    req.nh.nlmsg_len = NLMSG_ALIGN(req.nh.nlmsg_len) + RTA_ALIGN(rta->rta_len);

    if (sendNetlinkMessage(&req, req.nh.nlmsg_len)) {
        iface->metric = metric;
        if (state_callback_) {
            state_callback_(*iface);
        }
        return true;
    }

    return false;
}

bool NetworkInterfaceManager::refreshInterfaces() {
    readInterfaceInfo();
    return true;
}

void NetworkInterfaceManager::registerCallback(InterfaceCallback callback) {
    state_callback_ = callback;
}

void NetworkInterfaceManager::readInterfaceInfo() {
    // 读取网络接口信息
    interfaces_.clear();

    struct ifaddrs *ifaddr, *ifa;
    
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return;
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) {
            continue;
        }

        NetworkInterface iface;
        iface.name = ifa->ifa_name;
        
        // 设置接口状态
        if (ifa->ifa_flags & IFF_UP) {
            iface.state = InterfaceState::UP;
        } else {
            iface.state = InterfaceState::DOWN;
        }

        // 获取接口索引和 metric
        int ifindex = if_nametoindex(iface.name.c_str());
        iface.metric = ifindex;  // 使用 ifindex 作为 metric 的默认值

        // 判断接口类型
        if (strcmp(iface.name.c_str(), "lo") == 0) {
            iface.type = InterfaceType::LOOPBACK;
        } else if (strncmp(iface.name.c_str(), "wlan", 4) == 0 || 
                   strncmp(iface.name.c_str(), "wlp", 3) == 0) {
            iface.type = InterfaceType::WIFI;
        } else if (strncmp(iface.name.c_str(), "rmnet", 5) == 0 ||
                   strncmp(iface.name.c_str(), "ppp", 3) == 0) {
            iface.type = InterfaceType::MOBILE_DATA;
        } else if (strncmp(iface.name.c_str(), "eth", 3) == 0 ||
                   strncmp(iface.name.c_str(), "enp", 3) == 0) {
            iface.type = InterfaceType::ETHERNET;
        } else {
            iface.type = InterfaceType::OTHER;
        }

        // 获取 MAC 地址
        char mac_addr[18];
        if (if_nametoindex(iface.name.c_str()) > 0) {
            // 使用 netlink 获取 MAC 地址（简化实现）
            sprintf(mac_addr, "00:00:00:00:00:00");
            iface.mac_address = mac_addr;
        }

        // 获取 IP 地址
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr->sin_addr, ip_str, INET_ADDRSTRLEN);
            iface.ip_address = ip_str;

            // 获取子网掩码
            struct sockaddr_in *netmask = (struct sockaddr_in *)ifa->ifa_netmask;
            char netmask_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &netmask->sin_addr, netmask_str, INET_ADDRSTRLEN);
            iface.netmask = netmask_str;
        }

        interfaces_[iface.name] = iface;
    }

    freeifaddrs(ifaddr);
}

bool NetworkInterfaceManager::executeCommand(const std::string& command) {
    std::cout << "[NetworkInterfaceManager] Executing: " << command << std::endl;
    return true;
}

bool NetworkInterfaceManager::initNetlink() {
    nl_sock_ = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (nl_sock_ < 0) {
        perror("socket");
        return false;
    }

    struct sockaddr_nl addr;
    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = 0;

    if (bind(nl_sock_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(nl_sock_);
        nl_sock_ = -1;
        return false;
    }

    nl_initialized_ = true;
    return true;
}

void NetworkInterfaceManager::cleanupNetlink() {
    if (nl_sock_ >= 0) {
        close(nl_sock_);
        nl_sock_ = -1;
    }
    nl_initialized_ = false;
}

bool NetworkInterfaceManager::sendNetlinkMessage(void* buf, size_t len) {
    if (!nl_initialized_ || nl_sock_ < 0) {
        std::cerr << "Netlink socket not initialized" << std::endl;
        return false;
    }

    struct sockaddr_nl dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;

    // 设置 NLM_F_ACK 标志以接收确认
    if (sendto(nl_sock_, buf, len, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("sendto");
        return false;
    }

    // 接收响应
    char resp_buf[4096];
    ssize_t recv_len = recv(nl_sock_, resp_buf, sizeof(resp_buf), 0);
    if (recv_len < 0) {
        perror("recv");
        return false;
    }

    // 解析响应
    struct nlmsghdr* resp_nlh = (struct nlmsghdr*)resp_buf;
    if (resp_nlh->nlmsg_type == NLMSG_ERROR) {
        struct nlmsgerr* err = (struct nlmsgerr*)NLMSG_DATA(resp_nlh);
        if (err->error != 0) {
            std::cerr << "Netlink error: " << strerror(-err->error) << std::endl;
            return false;
        }
    }

    return true;
}

int NetworkInterfaceManager::getInterfaceIndex(const std::string& name) {
    int ifindex = if_nametoindex(name.c_str());
    if (ifindex == 0) {
        perror("if_nametoindex");
        return -1;
    }
    return ifindex;
}