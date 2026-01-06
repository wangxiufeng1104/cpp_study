# 组合复用原则 (Composite Reuse Principle, CRP)

## 原则定义

组合复用原则也称为"组合优于继承"（Favor Composition over Inheritance），是面向对象设计中的重要原则。其核心思想是：

> **在软件设计中，应该优先使用组合（Composition）而不是继承来达到代码复用的目的。**

换句话说，当需要在两个类之间建立关系时，如果能够使用组合，就不要使用继承。

## 组合 vs 继承

### 组合（Composition）

组合表示"has-a"关系（包含关系），即一个对象包含另一个对象。

**特点**：
- 运行时动态组合
- 松耦合
- 灵活性高
- 易于测试
- 避免类爆炸

**示例**：
```cpp
class Device {
private:
    std::vector<std::unique_ptr<Capability>> capabilities_;
    
public:
    void addCapability(std::unique_ptr<Capability> cap) {
        capabilities_.push_back(std::move(cap));
    }
};
```

### 继承（Inheritance）

继承表示"is-a"关系（派生关系），即一个类是另一个类的子类。

**特点**：
- 编译时静态组合
- 紧耦合
- 灵活性低
- 难以测试
- 可能导致类爆炸

**示例**：
```cpp
class SmartDevice : public NetworkCapability, 
                  public StorageCapability, 
                  public SensorCapability {
    // 继承所有能力
};
```

## 为什么需要组合复用原则？

### 1. 避免类爆炸

**问题场景**：
假设有3种能力（网络、存储、传感器），需要创建不同能力组合的设备：

使用继承（坏设计）：
- SmartDevice（网络+存储+传感器）
- IndustrialDevice（网络+传感器）
- IoTDevice（存储+传感器）
- NetworkStorageDevice（网络+存储）
- NetworkOnlyDevice（只有网络）
- StorageOnlyDevice（只有存储）
- SensorOnlyDevice（只有传感器）
- ...
- **结果**：类数量爆炸

使用组合（好设计）：
- Device基类
- 3种Capability实现
- 工厂类
- 管理器类
- **结果**：类数量固定，灵活组合

### 2. 提高灵活性（核心优势）

**继承的局限性（编译时静态）**：
- ❌ 能力组合在编译时确定，一旦编译就无法改变
- ❌ 运行时无法动态添加或移除能力
- ❌ 需要预先知道所有可能的能力组合
- ❌ 无法根据实际运行环境动态调整
- ❌ 无法支持用户自定义配置
- ❌ 无法实现插件化扩展

**组合的优势（运行时动态）**：
- ✅ **能力组合在运行时确定**：可以根据实际需求动态配置
- ✅ **运行时可以动态添加能力**：根据需要随时启用新功能
- ✅ **运行时可以动态移除能力**：根据需要随时禁用不需要的功能
- ✅ **灵活配置能力组合**：支持任意数量的能力组合方式
- ✅ **适应运行环境变化**：根据环境条件动态调整设备能力
- ✅ **支持热加载/热卸载**：支持插件系统的动态扩展

**设计阶段的重要依据**：

在设计阶段，组合的运行时灵活性应该作为设计决策的关键因素：

1. **需求不确定性**：
   - 如果需求可能变化，优先选择组合
   - 如果需要支持多种配置，优先选择组合
   - 如果需要支持用户自定义，优先选择组合

2. **运行时配置需求**：
   - 如果需要根据配置文件动态启用/禁用功能，选择组合
   - 如果需要根据环境变量动态调整能力，选择组合
   - 如果需要支持A/B测试，选择组合

3. **扩展性需求**：
   - 如果需要支持插件系统，选择组合
   - 如果需要支持热加载/热卸载，选择组合
   - 如果需要第三方扩展，选择组合

4. **灵活性 vs 性能权衡**：
   - 组合提供最大的灵活性，但可能有轻微的性能开销
   - 继承提供最佳性能，但缺乏灵活性
   - **在设计阶段就应该明确：灵活性是否比性能更重要**

**实际应用示例**：

```cpp
// 场景1：根据配置文件动态启用功能
void configureDevice(Device* device, const Config& config) {
    // 运行时根据配置添加能力
    if (config.enableNetwork) {
        device->addCapability(new NetworkCapability(config.networkType));
    }
    if (config.enableStorage) {
        device->addCapability(new StorageCapability(config.storageType, config.path));
    }
    if (config.enableSensors) {
        for (const auto& sensor : config.sensors) {
            device->addCapability(new SensorCapability(sensor.type));
        }
    }
}

// 场景2：根据运行环境动态调整
void adaptToEnvironment(Device* device, const Environment& env) {
    // 环境变化时动态添加能力
    if (env.isWiFiAvailable && !device->hasCapability("WiFi")) {
        device->addCapability(new WiFiCapability());
    }
    if (env.isWiredConnectionAvailable && device->hasCapability("WiFi")) {
        device->removeCapability("WiFi"); // 切换到有线连接
        device->addCapability(new EthernetCapability());
    }
}

// 场景3：用户自定义配置
void applyUserConfiguration(Device* device, const UserConfig& userConfig) {
    // 用户可以自由选择需要的功能
    for (const auto& feature : userConfig.enabledFeatures) {
        device->addCapability(createCapability(feature));
    }
}

// 场景4：插件系统
void loadPlugin(Device* device, const Plugin& plugin) {
    // 运行时动态加载插件
    auto capability = plugin.createCapability();
    device->addCapability(capability);
}

void unloadPlugin(Device* device, const std::string& pluginName) {
    // 运行时动态卸载插件
    device->removeCapability(pluginName);
}
```

**设计阶段的关键问题**：

在设计系统时，问自己以下问题：

1. **这个组件是否需要在运行时改变行为？**
   - 是 → 优先选择组合
   - 否 → 考虑继承

2. **是否需要支持用户自定义配置？**
   - 是 → 优先选择组合
   - 否 → 考虑继承

3. **是否需要支持插件或第三方扩展？**
   - 是 → 必须使用组合
   - 否 → 考虑继承

4. **需求是否会经常变化？**
   - 是 → 优先选择组合
   - 否 → 考虑继承

5. **性能是否比灵活性更重要？**
   - 是 → 可以考虑继承
   - 否 → 优先选择组合

**总结**：

组合的运行时灵活性是其最大的优势，这应该在设计阶段就作为重要的决策依据。当系统需要适应变化、支持动态配置、或者需求不确定时，组合是更好的选择。即使当前需求看起来很稳定，但为了未来的扩展性，也应该优先考虑组合。

记住：**在设计阶段，为未来的不确定性和灵活性需求做好规划，使用组合可以避免后期大规模重构。**

### 3. 降低耦合度

**继承的耦合**：
- 子类直接继承父类
- 子类与父类紧密耦合
- 修改父类可能影响所有子类
- 子类需要知道父类的所有接口

**组合的耦合**：
- 类之间通过接口依赖
- 松散耦合
- 修改一个类不影响其他类
- 只需要知道接口

### 4. 避免菱形继承问题

#### 什么是菱形继承问题？

菱形继承（Diamond Inheritance Problem）是多重继承中经典的问题，当一个类同时继承两个父类，而这两个父类又继承自同一个基类时，就会形成菱形继承结构。

```
        Base (基类)
       /    \
      /      \
  Parent1  Parent2 (父类)
      \      /
       \    /
      Child (子类)
```

#### 菱形继承导致的三个主要问题

**问题1：二义性（Ambiguity）**

当子类访问基类的成员时，编译器不知道应该访问哪个父类的基类成员。

```cpp
class Base {
public:
    int data;
    void method() { std::cout << "Base method" << std::endl; }
};

class Parent1 : public Base { /* ... */ };
class Parent2 : public Base { /* ... */ };

class Child : public Parent1, public Parent2 { /* ... */ };

// 问题：访问基类成员时出现二义性
Child child;
child.data = 10;      // ❌ 编译错误：二义性
child.method();         // ❌ 编译错误：二义性

// 必须明确指定使用哪个父类的基类
child.Parent1::data = 10;   // ✅ 可以
child.Parent2::data = 20;   // ✅ 可以
```

**问题2：数据冗余（Data Redundancy）**

子类中会有两份基类的副本，导致内存浪费和数据不一致。

```cpp
class Base {
public:
    int data;  // 这个数据会复制两份
};

class Parent1 : public Base { /* ... */ };
class Parent2 : public Base { /* ... */ };

class Child : public Parent1, public Parent2 { /* ... */ };

// 问题：子类包含两个Base实例
Child child;
child.Parent1::data = 10;  // 设置第一个Base的data
child.Parent2::data = 20;  // 设置第二个Base的data

// 两个data值不同，导致数据不一致
// 内存布局：
// Child对象:
//   Parent1::Base::data (value: 10)
//   Parent2::Base::data (value: 20)
```

**问题3：复杂性增加（Increased Complexity）**

菱形继承增加了类层次结构的复杂性，使代码难以理解和维护。

```cpp
// 复杂的菱形继承层次
class NetworkDevice { /* ... */ };
class StorageDevice { /* ... */ };
class SensorDevice { /* ... */ };
class SmartDevice : public NetworkDevice, public StorageDevice { /* ... */ };
class IoTDevice : public NetworkDevice, public SensorDevice { /* ... */ };
class AdvancedDevice : public SmartDevice, public IoTDevice { 
    // ❌ 复杂的多重菱形继承
    // ❌ 难以追踪继承关系
    // ❌ 难以理解对象布局
    // ❌ 难以调试和维护
};
```

#### 如何解决菱形继承问题？

**方案1：虚继承（Virtual Inheritance）**

使用虚继承可以解决菱形继承问题，但会带来新的问题。

```cpp
class Base {
public:
    int data;
    void method() { std::cout << "Base method" << std::endl; }
};

// 使用虚继承
class Parent1 : virtual public Base { /* ... */ };
class Parent2 : virtual public Base { /* ... */ };

class Child : public Parent1, public Parent2 { /* ... */ };

// ✅ 解决了二义性
Child child;
child.data = 10;   // ✅ 可以直接访问
child.method();      // ✅ 可以直接调用

// ✅ 解决了数据冗余
// 内存布局：
// Child对象:
//   Base::data (唯一的副本)
```

**虚继承的问题**：

1. **性能开销**：虚继承增加了额外的指针，增加了访问开销
2. **复杂性增加**：虚继承的实现复杂，理解困难
3. **构造函数复杂**：虚继承的构造顺序复杂
4. **调试困难**：虚继承的对象布局难以调试

```cpp
class Base {
public:
    Base() { std::cout << "Base constructor" << std::endl; }
    int data;
};

class Parent1 : virtual public Base {
public:
    Parent1() { std::cout << "Parent1 constructor" << std::endl; }
};

class Parent2 : virtual public Base {
public:
    Parent2() { std::cout << "Parent2 constructor" << std::endl; }
};

class Child : public Parent1, public Parent2 {
public:
    Child() { std::cout << "Child constructor" << std::endl; }
};

// 输出顺序复杂：
// Base constructor (最远基类先构造)
// Parent1 constructor
// Parent2 constructor
// Child constructor
```

**方案2：使用组合（推荐）**

使用组合可以完全避免菱形继承问题。

```cpp
class Base {
public:
    int data;
    void method() { std::cout << "Base method" << std::endl; }
};

class Parent1 { /* 不继承Base，而是包含Base */ };
class Parent2 { /* 不继承Base，而是包含Base */ };

class Child {
private:
    Parent1 parent1;
    Parent2 parent2;
    
public:
    // 通过组合访问
    void accessParent1() {
        parent1.method();
    }
    
    void accessParent2() {
        parent2.method();
    }
};

// ✅ 没有菱形继承问题
// ✅ 清晰的对象结构
// ✅ 易于理解和维护
// ✅ 没有额外的性能开销
```

#### 组合如何避免菱形继承问题？

**1. 没有继承层次**

```cpp
// 继承（有菱形问题）
class A { /* ... */ };
class B : public A { /* ... */ };
class C : public A { /* ... */ };
class D : public B, public C { /* ❌ 菱形继承 */ };

// 组合（没有菱形问题）
class A { /* ... */ };
class B { A a_; /* 组合 */ };
class C { A a_; /* 组合 */ };
class D { B b_; C c_; /* 组合 */ /* ✅ 没有问题 */ };
```

**2. 清晰的对象结构**

```cpp
// 嵌入式设备示例：使用组合避免菱形继承

// 继承方式（可能产生菱形继承）
class DeviceCapability { /* 基类 */ };
class NetworkCapability : public DeviceCapability { /* ... */ };
class StorageCapability : public DeviceCapability { /* ... */ };
class NetworkStorageCapability : public NetworkCapability, 
                                 public StorageCapability { 
    // ❌ 菱形继承问题
    // ❌ 二义性
    // ❌ 数据冗余
};

// 组合方式（没有菱形继承）
class DeviceCapability { /* ... */ };
class NetworkCapability : public DeviceCapability { /* ... */ };
class StorageCapability : public DeviceCapability { /* ... */ };

class Device {
private:
    std::vector<std::unique_ptr<DeviceCapability>> capabilities_;
    
public:
    void addCapability(std::unique_ptr<DeviceCapability> cap) {
        capabilities_.push_back(std::move(cap));
    }
    
    void useCapability(int index) {
        if (index < capabilities_.size()) {
            capabilities_[index]->method();
        }
    }
};

// ✅ 没有菱形继承问题
// ✅ 清晰的对象结构
// ✅ 灵活的能力组合
```

**3. 运行时灵活性**

```cpp
// 继承方式：能力组合在编译时确定
class SmartDevice : public NetworkCapability, 
                  public StorageCapability, 
                  public SensorCapability {
    // ❌ 编译时确定，无法改变
};

// 组合方式：能力组合在运行时确定
class Device {
    std::vector<std::unique_ptr<Capability>> capabilities_;
    
public:
    void addCapability(std::unique_ptr<Capability> cap) {
        capabilities_.push_back(std::move(cap));  // ✅ 运行时添加
    }
    
    void removeCapability(const std::string& type) {
        // ✅ 运行时移除
    }
};
```

#### 菱形继承问题的总结

| 方面 | 多重继承（有菱形问题） | 组合（无菱形问题） |
|------|------------------------|-------------------|
| 二义性 | ❌ 可能出现 | ✅ 不会出现 |
| 数据冗余 | ❌ 可能有多个副本 | ✅ 只有需要的副本 |
| 复杂性 | ❌ 继承层次复杂 | ✅ 对象结构清晰 |
| 性能 | ❌ 虚继承有开销 | ✅ 无额外开销 |
| 可维护性 | ❌ 难以维护 | ✅ 易于维护 |
| 灵活性 | ❌ 编译时确定 | ✅ 运行时确定 |

**关键要点**：

1. **菱形继承是多重继承的固有问题**：无论是否使用虚继承，多重继承都会增加复杂性
2. **虚继承可以解决但代价高昂**：虽然解决了菱形继承，但引入了新的复杂性
3. **组合完全避免菱形继承**：通过组合可以完全避免继承层次的问题
4. **组合提供更好的灵活性**：组合不仅避免菱形继承，还提供运行时灵活性

**设计建议**：

在设计系统时，如果发现需要使用多重继承，特别是可能形成菱形继承结构时，应该：

1. **重新设计类层次**：检查是否真的需要多重继承
2. **考虑使用组合**：优先选择组合而不是多重继承
3. **使用接口隔离**：定义接口，让类实现多个接口而不是多重继承
4. **使用组合+接口**：组合接口实现，而不是多重继承

**总结**：

菱形继承问题是多重继承的典型问题，它带来了二义性、数据冗余和复杂性增加等问题。虽然虚继承可以解决这些问题，但会带来新的复杂性和性能开销。组合不仅完全避免了菱形继承问题，还提供了更高的灵活性和更清晰的对象结构。因此，**组合优于继承**不仅是因为灵活性，也是为了避免菱形继承等问题。

### 5. 符合设计原则

**组合复用原则与其他原则的关系**：

1. **单一职责原则（SRP）**：
   - 继承：子类承担多个父类的职责
   - 组合：每个类职责单一

2. **开闭原则（OCP）**：
   - 继承：修改需要创建新类
   - 组合：扩展不需要修改现有代码

3. **依赖倒置原则（DIP）**：
   - 继承：依赖具体实现
   - 组合：依赖抽象接口

4. **里氏替换原则（LSP）**：
   - 继承：可能违反LSP
   - 组合：自然符合LSP

## 嵌入式设备场景演示

### 场景描述

在嵌入式Linux设备开发中，不同类型的设备需要不同的能力组合：

**能力类型**：
- **NetworkCapability**：网络通信能力（WiFi、以太网、蓝牙等）
- **StorageCapability**：存储能力（Flash、SD卡、eMMC等）
- **SensorCapability**：传感器能力（温度、湿度、运动传感器等）

**设备类型**：
- **SmartDevice**：智能设备（网络+存储+传感器）
- **IndustrialDevice**：工业设备（网络+传感器）
- **IoTDevice**：IoT设备（存储+传感器）

### 坏设计示例（使用继承）

#### 问题代码

```cpp
// 能力基类
class NetworkCapability { /* 网络功能 */ };
class StorageCapability { /* 存储功能 */ };
class SensorCapability { /* 传感器功能 */ };

// 智能设备：多重继承所有能力
class SmartDevice : public NetworkCapability, 
                  public StorageCapability, 
                  public SensorCapability {
    // 问题1：多重继承
    // 问题2：编译时确定能力组合
    // 问题3：运行时无法改变
};

// 工业设备：多重继承部分能力
class IndustrialDevice : public NetworkCapability, 
                       public SensorCapability {
    // 只继承部分能力
};

// IoT设备：多重继承部分能力
class IoTDevice : public StorageCapability,
                  public SensorCapability {
    // 只继承部分能力
};

// 问题：如果需要新的能力组合
class NetworkStorageDevice : public NetworkCapability,
                            public StorageCapability {
    // 必须创建新类
};
```

#### 问题分析

1. **类爆炸**：
   - 每种能力组合都需要创建新类
   - 3种能力，2³=8种组合，需要8个设备类
   - 如果有4种能力，需要16个类
   - 类数量随能力数量指数增长

2. **编译时确定**：
   - 能力组合在编译时确定
   - 运行时无法改变
   - 无法动态添加或移除能力

3. **紧耦合**：
   - 设备类与能力类紧密绑定
   - 修改能力类需要修改所有继承的设备类
   - 违反开闭原则

4. **违反单一职责原则**：
   - 设备类承担了多个父类的职责
   - 需要知道所有父类的接口
   - 职责不清晰

5. **多继承问题**：
   - 可能导致菱形继承问题
   - 继承层次复杂，难以维护
   - 命名冲突

6. **难以扩展**：
   - 添加新能力组合必须创建新类
   - 无法灵活配置设备能力
   - 代码重复

### 好设计示例（使用组合）

#### 优势代码

```cpp
// 能力接口
class ICapability {
public:
    virtual std::string getType() const = 0;
    virtual void execute() = 0;
};

// 能力实现
class NetworkCapability : public ICapability { /* 网络功能 */ };
class StorageCapability : public ICapability { /* 存储功能 */ };
class SensorCapability : public ICapability { /* 传感器功能 */ };

// 设备基类：使用组合
class Device {
private:
    std::vector<std::unique_ptr<ICapability>> capabilities_;
    
public:
    // 运行时动态添加能力
    void addCapability(std::unique_ptr<ICapability> capability) {
        capabilities_.push_back(std::move(capability));
    }
    
    // 运行时动态移除能力
    void removeCapability(const std::string& type) {
        // 实现移除逻辑
    }
    
    // 执行特定类型的能力
    void executeCapability(const std::string& type) {
        for (const auto& cap : capabilities_) {
            if (cap->getType() == type) {
                cap->execute();
            }
        }
    }
};

// 具体设备类
class SmartDevice : public Device { /* 智能设备特有逻辑 */ };
class IndustrialDevice : public Device { /* 工业设备特有逻辑 */ };
class IoTDevice : public Device { /* IoT设备特有逻辑 */ };

// 工厂类：创建设备并配置能力
class DeviceFactory {
public:
    static std::unique_ptr<Device> createSmartDevice(const std::string& id) {
        auto device = std::make_unique<SmartDevice>(id);
        device->addCapability(std::make_unique<NetworkCapability>("WiFi"));
        device->addCapability(std::make_unique<StorageCapability>("Flash", "/data"));
        device->addCapability(std::make_unique<SensorCapability>("Temp"));
        return device;
    }
};

// 管理器类：统一管理所有设备
class DeviceManager {
private:
    std::vector<std::unique_ptr<Device>> devices_;
    
public:
    void addDevice(std::unique_ptr<Device> device) {
        devices_.push_back(std::move(device));
    }
    
    void executeAllDevices() {
        for (const auto& device : devices_) {
            device->performTask();
        }
    }
};
```

#### 优势分析

1. **类数量少**：
   - 只需要1个Device基类
   - 只需要3个Capability实现类
   - 不需要为每种组合创建新类
   - 类数量固定

2. **运行时确定**：
   - 能力组合在运行时确定
   - 运行时可以动态添加能力
   - 运行时可以动态移除能力
   - 灵活配置设备能力

3. **松耦合**：
   - 设备类与能力类通过接口解耦
   - 修改能力类不影响设备类
   - 符合依赖倒置原则
   - 易于维护

4. **符合单一职责原则**：
   - Device类只负责管理能力
   - Capability类只负责具体功能
   - Factory类只负责创建设备
   - Manager类只负责管理设备
   - 职责清晰

5. **避免多继承问题**：
   - 没有多重继承
   - 类层次简单，易于维护
   - 没有菱形继承问题
   - 没有命名冲突

6. **易于扩展**：
   - 添加新能力只需实现ICapability
   - 不需要修改现有类
   - 符合开闭原则
   - 易于测试

## 设计对比

| 方面 | 继承（坏设计） | 组合（好设计） |
|------|-----------------|----------------|
| 灵活性 | 低（编译时确定） | 高（运行时确定） |
| 耦合度 | 高（紧耦合） | 低（松耦合） |
| 扩展性 | 难以扩展 | 易于扩展 |
| 类数量 | 爆炸式增长 | 固定数量 |
| 测试性 | 难以测试 | 易于测试 |
| 维护性 | 难以维护 | 易于维护 |
| 符合原则 | 违反多个原则 | 遵循多个原则 |

## 何时使用组合

### 1. 需要运行时动态改变行为

**场景**：
- 需要根据用户配置动态启用/禁用功能
- 需要根据环境条件动态调整能力
- 需要在运行时插件化扩展

**示例**：
```cpp
// 运行时添加网络能力
if (userConfig.enableNetwork) {
    device->addCapability(std::make_unique<NetworkCapability>("WiFi"));
}

// 运行时移除存储能力
if (!userConfig.enableStorage) {
    device->removeCapability("storage");
}
```

### 2. 需要灵活配置能力

**场景**：
- 产品有多种配置版本
- 用户可以自定义功能组合
- 需要支持A/B测试

**示例**：
```cpp
// 根据配置创建不同组合的设备
auto device = factory.createDevice(config.capabilities);
// config.capabilities 可以是任意组合
```

### 3. 需要动态添加/移除功能

**场景**：
- 插件系统
- 热加载/热卸载
- 动态功能扩展

**示例**：
```cpp
// 动态加载插件
void loadPlugin(const Plugin& plugin) {
    auto capability = plugin.createCapability();
    device->addCapability(std::move(capability));
}

// 动态卸载插件
void unloadPlugin(const std::string& pluginName) {
    device->removeCapability(pluginName);
}
```

### 4. 不确定是否适合继承

**场景**：
- 设计初期，不确定未来需求
- 需求经常变化
- 需要保持设计灵活性

**建议**：
- 当不确定时，优先选择组合
- 组合提供更高的灵活性
- 后续可以重构为继承（如果确实需要）

### 5. 避免类层次爆炸

**场景**：
- 有多种属性/能力需要组合
- 需要支持大量的组合方式
- 避免创建大量的子类

**示例**：
- 游戏开发中的角色属性系统
- GUI框架中的组件系统
- 微服务架构中的功能模块

## 何时使用继承

### 1. 明确的is-a关系

**场景**：
- 子类确实是一种特殊的父类
- 需要使用父类的地方都可以使用子类
- 符合里氏替换原则

**示例**：
```cpp
// 狗是一种动物（is-a关系）
class Dog : public Animal {
    // 狗继承动物的所有属性和行为
};

// 圆形是一种形状（is-a关系）
class Circle : public Shape {
    // 圆形继承形状的所有属性和行为
};
```

### 2. 需要利用多态

**场景**：
- 需要统一处理不同类型的对象
- 需要使用虚函数实现多态行为
- 需要运行时类型识别

**示例**：
```cpp
// 图形绘制系统
void drawAllShapes(const std::vector<Shape*>& shapes) {
    for (auto shape : shapes) {
        shape->draw();  // 多态调用
    }
}
```

### 3. 子类确实需要父类的所有功能

**场景**：
- 子类完全复用父类的实现
- 子类只添加或修改少量行为
- 不需要灵活配置

**示例**：
```cpp
// 异常类继承
class CustomException : public std::runtime_error {
    // 只需要添加自定义信息
    CustomException(const std::string& msg) : std::runtime_error(msg) {}
};
```

### 4. 层次结构清晰且稳定

**场景**：
- 领域模型清晰
- 继承层次稳定，不会频繁变化
- 团队对继承关系有共识

**示例**：
```cpp
// 文件系统层次
File -> Directory -> RegularFile
File -> Directory -> SymbolicLink
// 清晰且稳定的层次结构
```

## 实际应用场景

### 1. 游戏开发（组件系统）

**场景**：
游戏中的角色、道具、技能等需要灵活组合各种能力。

**组合方案**：
```cpp
class GameObject {
    std::vector<std::unique_ptr<Component>> components_;
};

class TransformComponent { /* 位置、旋转、缩放 */ };
class RenderComponent { /* 渲染 */ };
class PhysicsComponent { /* 物理模拟 */ };
class AIComponent { /* AI逻辑 */ };

// 创建角色
auto player = std::make_unique<GameObject>();
player->addComponent(std::make_unique<TransformComponent>());
player->addComponent(std::make_unique<RenderComponent>());
player->addComponent(std::make_unique<PhysicsComponent>());

// 创建静态物体
auto tree = std::make_unique<GameObject>();
tree->addComponent(std::make_unique<TransformComponent>());
tree->addComponent(std::make_unique<RenderComponent>());
// 不需要物理组件
```

### 2. 插件架构

**场景**：
应用程序需要支持插件扩展，运行时动态加载/卸载插件。

**组合方案**：
```cpp
class PluginManager {
    std::vector<std::unique_ptr<IPlugin>> plugins_;
};

class EditorPlugin : public IPlugin { /* 编辑器功能 */ };
class ExporterPlugin : public IPlugin { /* 导出功能 */ };
class UIPlugin : public IPlugin { /* UI功能 */ };

// 运行时加载插件
manager.loadPlugin("editor");
manager.loadPlugin("exporter");

// 运行时卸载插件
manager.unloadPlugin("editor");
```

### 3. 微服务架构

**场景**：
微服务需要灵活组合各种功能模块。

**组合方案**：
```cpp
class MicroService {
    std::vector<std::unique_ptr<ServiceModule>> modules_;
};

class AuthModule { /* 认证模块 */ };
class LoggingModule { /* 日志模块 */ };
class CacheModule { /* 缓存模块 */ };
class RateLimitModule { /* 限流模块 */ };

// 组合不同的服务
auto apiService = new MicroService();
apiService->addModule(new AuthModule());
apiService->addModule(new LoggingModule());
apiService->addModule(new RateLimitModule());

auto internalService = new MicroService();
internalService->addModule(new LoggingModule());
// 不需要认证和限流
```

### 4. UI框架设计

**场景**：
UI组件需要灵活组合各种装饰器、布局、事件处理。

**组合方案**：
```cpp
class Widget {
    std::vector<std::unique_ptr<Decorator>> decorators_;
};

class BorderDecorator { /* 边框 */ };
class ShadowDecorator { /* 阴影 */ };
class AnimationDecorator { /* 动画 */ };

// 组合不同的装饰器
auto button = new Button();
button->addDecorator(new BorderDecorator());
button->addDecorator(new ShadowDecorator());
```

### 5. 嵌入式系统设计

**场景**：
嵌入式设备需要根据硬件配置灵活组合各种驱动和能力。

**组合方案**：
```cpp
class EmbeddedDevice {
    std::vector<std::unique_ptr<Driver>> drivers_;
};

class NetworkDriver { /* 网络驱动 */ };
class DisplayDriver { /* 显示驱动 */ };
class SensorDriver { /* 传感器驱动 */ };

// 根据硬件配置组合驱动
auto device = new EmbeddedDevice();
if (hasWiFi) {
    device->addDriver(new WiFiDriver());
}
if (hasDisplay) {
    device->addDriver(new DisplayDriver());
}
```

## 最佳实践

### 1. 设计原则

- **优先组合，后考虑继承**：当不确定时，优先选择组合
- **保持接口简洁**：组合的接口应该简洁明了
- **使用工厂模式**：通过工厂模式创建组合对象
- **依赖抽象接口**：依赖接口而不是具体实现
- **单一职责**：每个组件职责单一

### 2. 实现建议

- **使用智能指针**：使用`std::unique_ptr`或`std::shared_ptr`管理生命周期
- **提供默认实现**：提供常用的组合配置
- **支持动态配置**：允许运行时修改组合
- **清晰的命名**：使用清晰的命名表达组合关系
- **文档化**：清晰地文档化组合关系和用法

### 3. 重构策略

从继承迁移到组合：

1. **识别继承关系**：找出需要重构的继承关系
2. **定义接口**：为需要组合的功能定义接口
3. **创建实现类**：创建接口的具体实现
4. **修改容器类**：将容器类改为使用组合
5. **逐步迁移**：逐步将功能迁移到组合方式
6. **测试验证**：确保功能正确性

### 4. 注意事项

- **避免过度设计**：不要为了组合而组合
- **考虑性能**：组合可能带来一定的性能开销
- **保持简单**：组合应该是简单的，不是复杂的
- **团队共识**：确保团队理解组合的设计思想

## CRP与其他设计原则的关系

### 1. 单一职责原则（SRP）

- **CRP符合SRP**：组合让每个类职责单一
- **继承违反SRP**：多重继承导致类承担多个职责

### 2. 开闭原则（OCP）

- **CRP符合OCP**：组合对扩展开放，对修改关闭
- **继承违反OCP**：添加新组合需要修改或创建新类

### 3. 里氏替换原则（LSP）

- **CRP符合LSP**：组合自然满足LSP
- **继承可能违反LSP**：不当的继承可能导致LSP违反

### 4. 接口隔离原则（ISP）

- **CRP符合ISP**：组合使用细粒度接口
- **继承违反ISP**：多重继承可能依赖不需要的接口

### 5. 依赖倒置原则（DIP）

- **CRP符合DIP**：组合依赖抽象接口
- **继承违反DIP**：继承依赖具体实现

## 编译和运行

```bash
# 编译
cd build
cmake ..
make crp_demo

# 运行
./bin/crp_demo
```

## 总结

组合复用原则是面向对象设计中的重要原则，它帮助我们：

1. **避免类爆炸**：减少不必要的类数量
2. **提高灵活性**：支持运行时动态配置
3. **降低耦合度**：通过接口实现松耦合
4. **提高可维护性**：类层次简单，易于维护
5. **提高可扩展性**：易于添加新功能
6. **符合设计原则**：符合多个SOLID原则

记住：**组合优于继承（Favor Composition over Inheritance）**

但也要注意：组合不是万能的，继承也有其适用的场景。关键是要根据具体情况选择合适的设计方式。

---

**相关资源**:
- [Design Patterns: Elements of Reusable Object-Oriented Software](https://en.wikipedia.org/wiki/Design_Patterns)
- [Favor Composition Over Inheritance](https://en.wikipedia.org/wiki/Composition_over_inheritance)
- [Object Composition](https://en.wikipedia.org/wiki/Object_composition)
