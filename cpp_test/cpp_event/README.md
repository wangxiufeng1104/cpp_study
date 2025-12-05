# C++ 高级事件驱动机制文档

## 概述

本文档详细介绍了一个高性能、功能丰富的C++事件驱动系统。该系统采用了发布-订阅模式，支持事件优先级、回调管理、异常处理等企业级特性。

## 系统架构

### 核心组件

1. **EventType** - 事件类型枚举
2. **EventPriority** - 事件优先级枚举
3. **EventData** - 事件数据结构
4. **EventCallbackWrapper** - 事件回调包装器
5. **EventManager** - 事件管理器核心类
6. **全局兼容性函数** - 向后兼容接口

### 类图结构

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   EventData     │    │ EventCallback    │    │  EventManager   │
├─────────────────┤    │   Wrapper        │    ├─────────────────┤
│ + type: enum    │    ├──────────────────┤    │ + addCallback() │
│ + message: str  │    │ + callback: fn   │    │ + removeCallback│
│ + value: double │    │ + id: size_t     │    │ + notifyEvent() │
│ + priority: enum│    │ + name: string   │    │ + setEnabled()  │
│ + timestamp     │    │ + minPriority:   │    │ + statistics()  │
│ + source: str   │    │ + enabled: bool  │    └─────────────────┘
└─────────────────┘    └──────────────────┘
```

## 数据结构详解

### EventType 枚举

```cpp
enum class EventType { 
    TemperatureChanged,    // 温度变化事件
    ModeChanged,          // 模式变化事件
    SystemStateChanged,   // 系统状态变化事件
    ErrorOccurred        // 错误发生事件
};
```

### EventPriority 枚举

```cpp
enum class EventPriority { 
    Low = 0,      // 低优先级
    Normal = 1,    // 普通优先级
    High = 2,      // 高优先级
    Critical = 3   // 关键优先级
};
```

### EventData 结构体

```cpp
struct EventData {
    EventType type;                           // 事件类型
    std::string message;                       // 事件消息
    double value;                             // 事件数值
    EventPriority priority;                    // 事件优先级
    std::chrono::system_clock::time_point timestamp; // 时间戳
    std::string source;                       // 事件源

    EventData(EventType t, const std::string& msg = "", 
              double val = 0.0, EventPriority prio = EventPriority::Normal, 
              const std::string& src = "Unknown");
};
```

## EventManager 类详解

### 私有成员

- `std::map<EventType, std::vector<EventCallbackWrapper>> eventCallbacks` - 事件回调存储
- `std::unordered_set<CallbackID> usedIds` - 已使用的ID集合
- `CallbackID nextId` - 下一个可用ID
- `bool debugMode` - 调试模式开关

### 公共接口

#### 1. 调试控制

```cpp
void setDebugMode(bool enabled);
```

#### 2. 回调注册

```cpp
// 按事件类型注册回调
CallbackID addEventCallback(EventType type, EventCallback callback, 
                           const std::string& name = "", 
                           EventPriority minPriority = EventPriority::Low);

// 为所有事件类型注册回调
CallbackID addEventCallback(EventCallback callback, const std::string& name = "",
                           EventPriority minPriority = EventPriority::Low);
```

#### 3. 回调管理

```cpp
// 移除特定回调
bool removeEventCallback(CallbackID id);

// 启用/禁用特定回调
bool setCallbackEnabled(CallbackID id, bool enabled);

// 移除所有回调
void removeAllCallbacks();

// 移除特定类型的所有回调
void removeCallbacks(EventType type);
```

#### 4. 事件通知

```cpp
void notifyEvent(const EventData& event);
```

#### 5. 统计信息

```cpp
void printStatistics() const;
```

## 使用指南

### 基本使用

#### 1. 创建事件处理器

```cpp
class TemperatureMonitor {
public:
    void onTemperatureChanged(const EventData& event) {
        if (event.type == EventType::TemperatureChanged) {
            std::cout << "Temperature: " << event.value << "°C" << std::endl;
        }
    }
};

TemperatureMonitor monitor;
```

#### 2. 注册回调函数

```cpp
// 注册通用处理器
CallbackID handlerId = globalEventManager.addEventCallback(
    EventType::TemperatureChanged, 
    handleEvent, 
    "TemperatureHandler"
);

// 注册类方法
CallbackID monitorId = globalEventManager.addEventCallback(
    EventType::TemperatureChanged,
    [&monitor](const EventData& event) {
        monitor.onTemperatureChanged(event);
    },
    "TemperatureMonitor",
    EventPriority::Normal
);
```

#### 3. 触发事件

```cpp
// 创建并触发事件
EventData tempEvent(EventType::TemperatureChanged, 
                   "High temperature detected", 
                   35.5, 
                   EventPriority::High, 
                   "SensorA");
globalEventManager.notifyEvent(tempEvent);

// 或者直接触发
globalEventManager.notifyEvent(EventData(
    EventType::ModeChanged, 
    "Switch to auto mode", 
    1.0, 
    EventPriority::Normal, 
    "ModeController"
));
```

### 高级功能

#### 1. 优先级过滤

```cpp
// 只处理高优先级以上的事件
CallbackID criticalHandler = globalEventManager.addEventCallback(
    EventType::ErrorOccurred,
    [](const EventData& event) {
        std::cout << "CRITICAL: " << event.message << std::endl;
    },
    "CriticalHandler",
    EventPriority::High  // 只处理High和Critical优先级的事件
);
```

#### 2. 动态控制

```cpp
// 禁用回调
globalEventManager.setCallbackEnabled(monitorId, false);

// 重新启用
globalEventManager.setCallbackEnabled(monitorId, true);

// 移除回调
bool removed = globalEventManager.removeEventCallback(handlerId);
```

#### 3. 调试模式

```cpp
// 启用调试模式，查看详细的注册和操作日志
globalEventManager.setDebugMode(true);
```

## 性能优化特性

### 1. 事件类型分组存储

- 使用 `std::map<EventType, std::vector<EventCallbackWrapper>>` 按类型分组存储
- 事件通知时只遍历对应类型的回调列表
- 避免了不必要的函数调用开销

### 2. 优先级过滤

- 回调注册时可设置最小处理优先级
- 事件通知时进行优先级检查
- 低优先级事件不会触发高优先级要求的处理器

### 3. 唯一ID管理

- 每个回调函数都有唯一的 `CallbackID`
- 支持精确的启用/禁用和移除操作
- 高效的ID分配和回收机制

### 4. 异常隔离

- 回调函数执行时的异常被捕获并记录
- 单个回调的异常不会影响其他处理器的执行
- 保证系统的稳定性和可靠性

## 兼容性接口

为了保持向后兼容，系统提供了全局函数接口：

```cpp
// 向后兼容的全局函数
void addEventCallback(EventType type, EventCallback callback);
void addEventCallback(EventCallback callback);
void removeEventCallbacks();
void removeEventCallbacks(EventType type);
void notifyEvent(const EventData& event);
```

## 最佳实践

### 1. 回调函数设计

```cpp
// 好的做法：检查事件类型，处理异常
void goodHandler(const EventData& event) {
    try {
        if (event.type == EventType::TemperatureChanged) {
            // 处理温度事件
            processTemperature(event.value);
        }
    } catch (const std::exception& e) {
        std::cerr << "Handler error: " << e.what() << std::endl;
    }
}

// 避免的做法：不检查类型，不处理异常
void badHandler(const EventData& event) {
    // 直接使用可能导致错误
    auto temp = event.value;  // 可能不是温度事件
}
```

### 2. 优先级使用建议

```cpp
// 系统关键事件：Critical
globalEventManager.notifyEvent(EventData(
    EventType::ErrorOccurred, 
    "System crash", 
    0, 
    EventPriority::Critical, 
    "SystemCore"
));

// 重要业务事件：High
globalEventManager.notifyEvent(EventData(
    EventType::TemperatureChanged, 
    "Overheating warning", 
    45.0, 
    EventPriority::High, 
    "TemperatureSensor"
));

// 常规业务事件：Normal
globalEventManager.notifyEvent(EventData(
    EventType::ModeChanged, 
    "User mode switch", 
    1.0, 
    EventPriority::Normal, 
    "UIController"
));

// 调试和日志事件：Low
globalEventManager.notifyEvent(EventData(
    EventType::SystemStateChanged, 
    "Debug info", 
    0, 
    EventPriority::Low, 
    "DebugModule"
));
```

### 3. 资源管理

```cpp
class MyComponent {
private:
    std::vector<CallbackID> callbackIds_;
    
public:
    void initialize() {
        // 注册回调并保存ID
        CallbackID id1 = globalEventManager.addEventCallback(
            EventType::TemperatureChanged, 
            [this](const EventData& e) { onTempChange(e); },
            "MyComponent_Temp"
        );
        callbackIds_.push_back(id1);
    }
    
    ~MyComponent() {
        // 析构时清理所有回调
        for (auto id : callbackIds_) {
            globalEventManager.removeEventCallback(id);
        }
    }
};
```

## 示例程序

系统包含一个完整的示例程序，演示了所有主要功能：

1. **事件注册** - 演示不同类型的回调注册
2. **优先级处理** - 展示优先级过滤效果
3. **动态管理** - 演示启用/禁用和移除操作
4. **统计信息** - 显示系统运行状态
5. **异常处理** - 展示异常隔离机制

## 扩展建议

### 1. 线程安全

当前实现是单线程的，可以扩展为线程安全版本：

```cpp
class ThreadSafeEventManager : public EventManager {
private:
    std::mutex callbacksMutex_;
    
public:
    void notifyEvent(const EventData& event) override {
        std::lock_guard<std::mutex> lock(callbacksMutex_);
        EventManager::notifyEvent(event);
    }
    
    // 其他方法也需要加锁...
};
```

### 2. 异步事件处理

```cpp
class AsyncEventManager : public EventManager {
private:
    std::thread workerThread_;
    std::queue<EventData> eventQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCV_;
    bool running_ = true;
    
public:
    void notifyEventAsync(const EventData& event) {
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            eventQueue_.push(event);
        }
        queueCV_.notify_one();
    }
    
private:
    void workerFunction() {
        while (running_) {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCV_.wait(lock, [this] { return !eventQueue_.empty() || !running_; });
            
            while (!eventQueue_.empty()) {
                auto event = eventQueue_.front();
                eventQueue_.pop();
                lock.unlock();
                
                EventManager::notifyEvent(event);
                
                lock.lock();
            }
        }
    }
};
```

### 3. 事件过滤器

```cpp
class EventFilter {
public:
    virtual bool shouldProcess(const EventData& event) const = 0;
};

class TimeRangeFilter : public EventFilter {
private:
    std::chrono::system_clock::time_point start_;
    std::chrono::system_clock::time_point end_;
    
public:
    bool shouldProcess(const EventData& event) const override {
        return event.timestamp >= start_ && event.timestamp <= end_;
    }
};
```

## 总结

本C++事件驱动机制提供了以下核心优势：

1. **高性能** - 事件类型分组和优先级过滤减少无效调用
2. **功能丰富** - 支持命名、启用控制、异常处理等企业级特性
3. **易于使用** - 简洁的API设计和向后兼容性
4. **可扩展** - 模块化设计便于功能扩展
5. **调试友好** - 详细的日志和统计信息

该系统适用于需要复杂事件处理的各种C++应用程序，从简单的桌面应用到高性能的服务器系统。
