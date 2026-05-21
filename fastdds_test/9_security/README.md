# 安全机制示例 - FastDDS DDS Security

本示例演示如何配置和使用 FastDDS 的 DDS Security 安全特性，实现加密通信。

## 概念说明

### DDS Security 规范

DDS Security 是 OMG DDS 规范的安全扩展，提供以下安全服务：

| 服务 | 说明 |
|------|------|
| **Authentication** | 身份验证 - 验证参与者身份 |
| **AccessControl** | 访问控制 - 控制对主题的访问权限 |
| **Cryptographic** | 加密服务 - 数据加密和解密 |
| **Logging** | 安全日志 - 记录安全相关事件 |
| **DataTagging** | 数据标记 - 数据分类和标记 |

### 安全架构

```
┌─────────────────────────────────────────────────────┐
│                  DDS Application                     │
├─────────────────────────────────────────────────────┤
│                  DDS Security                        │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐            │
│  │  Auth    │ │ Access   │ │  Crypto  │            │
│  │ Plugin   │ │ Control  │ │ Plugin   │            │
│  └──────────┘ └──────────┘ └──────────┘            │
├─────────────────────────────────────────────────────┤
│                  DDS / RTPS                          │
└─────────────────────────────────────────────────────┘
```

## 文件结构

```
9_security/
├── CMakeLists.txt      # 构建配置
├── publisher.cpp       # 安全发布者实现
├── subscriber.cpp      # 安全订阅者实现
└── README.md           # 本文件
```

## 证书文件

DDS Security 需要以下证书文件：

| 文件 | 说明 |
|------|------|
| `cacert.pem` | CA 证书 - 用于验证身份证书 |
| `cert.pem` | 身份证书 - 参与者的身份证明 |
| `key.pem` | 私钥 - 与身份证书配对 |
| `governance.p7s` | 治理规则 - 定义安全策略 |
| `permissions.p7s` | 权限规则 - 定义访问权限 |

### 证书目录结构

```
certs/
├── cacert.pem
├── cert.pem
├── key.pem
├── governance.p7s
└── permissions.p7s
```

## 安全配置

### 身份验证配置

```cpp
DomainParticipantQos pqos;

// 身份验证插件
pqos.properties().properties().emplace_back(
    "dds.sec.auth.plugin", "builtin.PKI-DH");

// CA 证书
pqos.properties().properties().emplace_back(
    "dds.sec.auth.builtin.PKI-DH.identity_ca", 
    "file:" + certs_path + "/cacert.pem");

// 身份证书
pqos.properties().properties().emplace_back(
    "dds.sec.auth.builtin.PKI-DH.identity_certificate",
    "file:" + certs_path + "/cert.pem");

// 私钥
pqos.properties().properties().emplace_back(
    "dds.sec.auth.builtin.PKI-DH.private_key",
    "file:" + certs_path + "/key.pem");
```

### 加密配置

```cpp
// 加密插件
pqos.properties().properties().emplace_back(
    "dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");
```

### 访问控制配置

```cpp
// 访问控制插件
pqos.properties().properties().emplace_back(
    "dds.sec.access.plugin", "builtin.Access-Permissions");

// 权限 CA
pqos.properties().properties().emplace_back(
    "dds.sec.access.builtin.Access-Permissions.permissions_ca",
    "file:" + certs_path + "/cacert.pem");

// 治理规则
pqos.properties().properties().emplace_back(
    "dds.sec.access.builtin.Access-Permissions.governance",
    "file:" + certs_path + "/governance.p7s");

// 权限规则
pqos.properties().properties().emplace_back(
    "dds.sec.access.builtin.Access-Permissions.permissions",
    "file:" + certs_path + "/permissions.p7s");
```

## 编译

在项目根目录执行:

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

**注意**: 需要编译时启用 SECURITY 选项。

## 生成证书

### 使用 FastDDS 提供的工具

FastDDS 提供了证书生成脚本：

```bash
# 设置环境变量
export FASTDDS_HOME=/path/to/fastdds

# 运行证书生成脚本
$FASTDDS_HOME/tools/security/certificates/generate_certs.sh ./certs
```

### 手动生成证书

```bash
# 1. 生成 CA 私钥
openssl genrsa -out ca_key.pem 2048

# 2. 生成 CA 证书
openssl req -new -x509 -days 365 -key ca_key.pem -out cacert.pem

# 3. 生成参与者私钥
openssl genrsa -out key.pem 2048

# 4. 生成证书签名请求 (CSR)
openssl req -new -key key.pem -out cert.csr

# 5. 使用 CA 签名生成证书
openssl x509 -req -days 365 -in cert.csr -CA cacert.pem -CAkey ca_key.pem \
    -CAcreateserial -out cert.pem
```

### 治理规则示例 (governance.xml)

```xml
<?xml version="1.0" encoding="UTF-8"?>
<dds xmlns="https://www.omg.org/spec/DDS-SECURITY/20170901/dds_sec_xml">
    <domain_access_rules>
        <domain_rule>
            <domains>
                <id_range>
                    <min>0</min>
                    <max>230</max>
                </id_range>
            </domains>
            <allow_unauthenticated_participants>false</allow_unauthenticated_participants>
            <enable_join_access_control>true</enable_join_access_control>
            <topic_access_rules>
                <topic_rule>
                    <topics>
                        <topic>*</topic>
                    </topics>
                    <enable_discovery_protection>true</enable_discovery_protection>
                    <enable_liveliness_protection>true</enable_liveliness_protection>
                    <enable_read_access_control>true</enable_read_access_control>
                    <enable_write_access_control>true</enable_write_access_control>
                    <metadata_protection_kind>ENCRYPT</metadata_protection_kind>
                    <data_protection_kind>ENCRYPT</data_protection_kind>
                </topic_rule>
            </topic_access_rules>
        </domain_rule>
    </domain_access_rules>
</dds>
```

### 权限规则示例 (permissions.xml)

```xml
<?xml version="1.0" encoding="UTF-8"?>
<dds xmlns="https://www.omg.org/spec/DDS-SECURITY/20170901/dds_sec_xml">
    <permissions>
        <grant name="participant_permission">
            <subject_name>CN=participant</subject_name>
            <validity>
                <not_before>2023-01-01T00:00:00</not_before>
                <not_after>2030-01-01T00:00:00</not_after>
            </validity>
            <allow_rule>
                <domains>
                    <id_range>
                        <min>0</min>
                        <max>230</max>
                    </id_range>
                </domains>
                <publish>
                    <topics>
                        <topic>*</topic>
                    </topics>
                </publish>
                <subscribe>
                    <topics>
                        <topic>*</topic>
                    </topics>
                </subscribe>
            </allow_rule>
        </grant>
    </permissions>
</dds>
```

## 运行

### 终端 1 - 启动安全订阅者

```bash
./9_security_subscriber ./certs
```

### 终端 2 - 启动安全发布者

```bash
./9_security_publisher ./certs
```

## 预期输出

### 发布者输出

```
========================================
安全机制 发布者示例
========================================
安全发布者初始化成功!

等待安全订阅者连接...
匹配到安全订阅者 (匹配数: 1)
[2026-03-16 16:30:00] 已发布(加密): Secure Message #0
[2026-03-16 16:30:01] 已发布(加密): Secure Message #1
...
```

### 订阅者输出

```
========================================
安全机制 订阅者示例
========================================
安全订阅者初始化成功!

匹配到安全发布者 (匹配数: 1)
[2026-03-16 16:30:00] 收到(解密): Secure Message #0
[2026-03-16 16:30:01] 收到(解密): Secure Message #1
...
```

## 错误处理

### 证书配置错误

```
错误: 无法创建安全参与者 (检查证书配置)

提示: 安全机制需要正确的证书文件。
请确保以下文件存在:
  - cacert.pem (CA证书)
  - cert.pem (身份证书)
  - key.pem (私钥)
  - governance.p7s (治理规则)
  - permissions.p7s (权限规则)
```

### 常见错误排查

| 错误 | 可能原因 | 解决方法 |
|------|----------|----------|
| 证书验证失败 | 证书过期或不匹配 | 检查证书有效期和签名 |
| 权限被拒绝 | 权限规则限制 | 检查 permissions.xml |
| 握手失败 | 私钥不匹配 | 确认私钥与证书配对 |
| 连接超时 | 证书文件路径错误 | 检查文件路径和权限 |

## 关键代码说明

### 安全发布者初始化

```cpp
bool init(const std::string& certs_path)
{
    DomainParticipantQos pqos;
    pqos.name("Secure_Publisher");

    // 配置身份验证
    pqos.properties().properties().emplace_back(
        "dds.sec.auth.plugin", "builtin.PKI-DH");
    pqos.properties().properties().emplace_back(
        "dds.sec.auth.builtin.PKI-DH.identity_ca", 
        "file:" + certs_path + "/cacert.pem");
    pqos.properties().properties().emplace_back(
        "dds.sec.auth.builtin.PKI-DH.identity_certificate",
        "file:" + certs_path + "/cert.pem");
    pqos.properties().properties().emplace_back(
        "dds.sec.auth.builtin.PKI-DH.private_key",
        "file:" + certs_path + "/key.pem");

    // 配置加密
    pqos.properties().properties().emplace_back(
        "dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");

    // 配置访问控制
    pqos.properties().properties().emplace_back(
        "dds.sec.access.plugin", "builtin.Access-Permissions");
    pqos.properties().properties().emplace_back(
        "dds.sec.access.builtin.Access-Permissions.permissions_ca",
        "file:" + certs_path + "/cacert.pem");
    pqos.properties().properties().emplace_back(
        "dds.sec.access.builtin.Access-Permissions.governance",
        "file:" + certs_path + "/governance.p7s");
    pqos.properties().properties().emplace_back(
        "dds.sec.access.builtin.Access-Permissions.permissions",
        "file:" + certs_path + "/permissions.p7s");

    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (!participant_)
    {
        std::cerr << "错误: 无法创建安全参与者" << std::endl;
        return false;
    }
    
    // ... 后续 Topic, Publisher, DataWriter 创建
}
```

## 安全最佳实践

1. **私钥保护**: 私钥文件应设置严格的访问权限
   ```bash
   chmod 600 key.pem
   ```

2. **证书更新**: 定期更新即将过期的证书

3. **最小权限原则**: 在权限规则中只授予必要的权限

4. **审计日志**: 启用安全日志进行审计
   ```cpp
   pqos.properties().properties().emplace_back(
       "dds.sec.logging.plugin", "builtin.Logging");
   ```

5. **生产环境**: 使用正规 CA 签发的证书

## 注意事项

1. **性能影响**: 加密会增加 CPU 使用和延迟
2. **证书管理**: 需要妥善管理和更新证书
3. **调试困难**: 加密数据难以直接查看
4. **配置复杂**: 安全配置较为复杂，需要仔细检查

## 下一步

- 回顾 `1_hello_world` 理解基础通信流程
- 查看 `4_qos_config` 了解 QoS 配置
- 查看 `6_tcp_transport` 了解 TCP 传输