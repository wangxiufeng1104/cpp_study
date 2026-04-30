# C++ 设计原则参考手册

本文档是设计原则的精简参考，用于在设计代码时快速查阅。所有原则均以嵌入式Linux设备开发为背景。

---

## 1. 单一职责原则 (SRP)

> **一个类应该有且仅有一个引起它变化的原因。**

- 每个类只负责一项职责
- 降低复杂性，提高可维护性和可测试性
- **识别方法**：问自己"这个类为什么要改变？"如果有多个原因，就需要拆分

```cpp
// 坏：职责耦合
class SensorDevice {
    float readTemperature();   // 传感器读取
    bool connectMQTT();        // MQTT通信
    void logInfo();            // 日志记录
};

// 好：职责分离
class SensorReader { SensorData read(); };
class MqttPublisher { bool publish(...); };
class DeviceLogger { void log(...); };
```

---

## 2. 开闭原则 (OCP)

> **软件实体应该对扩展开放，对修改关闭。**

- 添加新功能通过扩展（新增类）实现，不修改现有代码
- 使用抽象接口 + 多态 + 依赖注入实现
- **核心技巧**：用抽象隔离变化点，用工厂模式创建对象

```cpp
// 坏：switch-case，添加协议需要修改类
class DeviceCommunicator {
    bool connect() {
        switch (protocol_) { case MQTT: ...; case HTTP: ...; }
    }
};

// 好：多态，添加协议只需新增类
class IProtocolCommunicator { virtual bool connect() = 0; };
class MqttCommunicator : public IProtocolCommunicator { ... };
class HttpCommunicator : public IProtocolCommunicator { ... };
```

---

## 3. 里氏替换原则 (LSP)

> **子类对象必须能够替换所有父类对象，而不会破坏程序的正确性。**

- 子类必须遵守父类的契约（前置条件、后置条件、不变量）
- 子类不能抛出父类未声明的异常
- 子类不能改变方法返回值的含义
- **检测方法**：为父类编写契约测试，所有子类都应通过

```cpp
// 契约：connect()返回true表示成功，disconnect()后isConnected()返回false
class Communicator {
    virtual bool connect() = 0;
    virtual bool send(const std::string& data) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
};

// 所有子类必须严格遵守契约
class MqttCommunicator : public Communicator {
    void disconnect() override { connected_ = false; } // 必须满足后置条件
};
```

---

## 4. 接口隔离原则 (ISP)

> **客户端不应该被迫依赖它们不使用的接口。**

- 接口应该小而专注，不要设计臃肿的大接口
- 类只实现它真正需要的接口
- **拆分依据**：按职责/客户端需求拆分接口

```cpp
// 坏：臃肿接口，传感器被迫实现9个方法
class IDevice {
    virtual double readSensor() = 0;
    virtual void executeCommand(...) = 0;
    virtual void display(...) = 0;
    virtual bool connectNetwork(...) = 0;
    // ... 更多不需要的方法
};

// 好：细分接口，每个类只实现需要的
class ISensor { virtual double readValue() = 0; };
class IActuator { virtual void execute(...) = 0; };
class IDisplay { virtual void show(...) = 0; };
class INetwork { virtual bool connect(...) = 0; virtual bool send(...) = 0; };

class TemperatureSensor : public ISensor { /* 只实现readValue */ };
```

---

## 5. 依赖倒置原则 (DIP)

> **高层模块不应该依赖低层模块，两者都应该依赖抽象。**

- 高层模块定义抽象接口，低层模块实现接口
- 通过依赖注入（构造函数/setter）传递依赖
- 使用Mock对象进行单元测试

```cpp
// 坏：高层直接依赖低层
class DeviceManager {
    FileStorage storage_;  // 直接依赖具体实现
};

// 好：都依赖抽象
class IStorage { virtual bool save(...) = 0; virtual bool load(...) = 0; };
class FileStorage : public IStorage { ... };
class DatabaseStorage : public IStorage { ... };

class DeviceManager {
    IStorage& storage_;  // 依赖抽象
    DeviceManager(IStorage& storage) : storage_(storage) {}
};
```

---

## 6. 迪米特法则 (LoD)

> **只与你的"直接朋友"通信，不要跟"陌生人"说话。**

- "直接朋友"：this、成员变量、方法参数、方法内创建的对象
- "陌生人"：直接朋友的内部对象（链式调用 `a.b.c()` 违反LoD）
- **核心做法**：在中间对象中封装高层接口，避免深层调用链

```cpp
// 坏：深层调用链，访问"陌生人"
void Monitor::report() {
    Sensor* sensor = controller_->getSensor(0);  // 陌生人
    sensor->getData();                             // 与陌生人通信
    controller_->getNetwork()->send(data);         // 与陌生人通信
}

// 好：只与直接朋友通信
void Monitor::report() {
    controller_->reportSensorData(0);  // 只调用直接朋友的高层接口
}
```

---

## 7. 组合复用原则 (CRP)

> **优先使用组合（Composition）而不是继承（Inheritance）来达到代码复用的目的。**

- 组合 = "has-a"关系，继承 = "is-a"关系
- 组合提供运行时灵活性（动态添加/移除能力），继承在编译时确定
- 组合避免菱形继承问题、类爆炸问题
- **使用组合的场景**：需要运行时动态改变行为、灵活配置、插件化扩展
- **使用继承的场景**：明确的is-a关系、需要多态、层次结构稳定

```cpp
// 坏：继承导致类爆炸，编译时确定能力组合
class SmartDevice : public NetworkCapability, public StorageCapability, public SensorCapability { };
class IndustrialDevice : public NetworkCapability, public SensorCapability { };
// 3种能力需要8个类...

// 好：组合，运行时动态配置
class Device {
    std::vector<std::unique_ptr<ICapability>> capabilities_;
public:
    void addCapability(std::unique_ptr<ICapability> cap);
    void removeCapability(const std::string& type);
};
// 只需1个Device基类 + N个Capability实现
```

---

## 原则间的关系

| 原则 | 核心关注点 | 与其他原则的关系 |
|------|-----------|-----------------|
| SRP | 类的职责单一 | 为OCP、ISP奠定基础 |
| OCP | 对扩展开放，对修改关闭 | 依赖SRP、DIP、LSP实现 |
| LSP | 子类安全替换父类 | 是OCP的基础，需要ISP支持 |
| ISP | 接口小而专注 | 促进SRP、支持DIP |
| DIP | 依赖抽象而非具体 | 是实现OCP的关键 |
| LoD | 减少对象间依赖 | 补充DIP，进一步降低耦合 |
| CRP | 组合优于继承 | 符合SRP、OCP、DIP、LSP、ISP |

---

## 设计决策速查

1. **不确定时，优先选择组合**（灵活性更高）
2. **识别变化点，用抽象隔离**（OCP）
3. **接口设计从客户端角度出发**（ISP）
4. **依赖注入，不要直接创建依赖**（DIP）
5. **提供高层接口，封装内部实现**（LoD）
6. **一个类做一件事，做好一件事**（SRP）
7. **继承前先问：子类能否完全替换父类？**（LSP）

---

## 编译和运行

```bash
cd build
cmake ..
make
./bin/single_responsibility_demo   # SRP演示
./bin/ocp_demo                     # OCP演示
./bin/lsp_demo                     # LSP演示
./bin/isp_demo                     # ISP演示
./bin/dip_demo                     # DIP演示
./bin/lod_demo                     # LoD演示
./bin/crp_demo                     # CRP演示
```
