#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

enum class EventType {
    TemperatureChanged,
    ModeChanged,
    SystemStateChanged,
    ErrorOccurred
};

// 事件优先级
enum class EventPriority {
    Low = 0,
    Normal = 1,
    High = 2,
    Critical = 3
};

struct EventData {
    EventType type;
    std::string message;
    double value;
    EventPriority priority;
    std::chrono::system_clock::time_point timestamp;
    std::string source;

    EventData(EventType t, const std::string& msg = "", double val = 0.0, EventPriority prio = EventPriority::Normal,
        const std::string& src = "Unknown")
    : type(t),
      message(msg),
      value(val),
      priority(prio),
      timestamp(std::chrono::system_clock::now()),
      source(src) {}
};

using EventCallback = std::function<void(const EventData&)>;
using CallbackID = size_t;

// 事件处理器包装器
struct EventCallbackWrapper {
    EventCallback callback;
    CallbackID id;
    std::string name;
    EventPriority minPriority; // 只处理优先级不低于此值的事件
    bool enabled;

    EventCallbackWrapper(
        EventCallback cb, CallbackID i, const std::string& n = "", EventPriority minPrio = EventPriority::Low)
    : callback(cb),
      id(i),
      name(n),
      minPriority(minPrio),
      enabled(true) {}
};

class EventManager {
private:
    std::map<EventType, std::vector<EventCallbackWrapper>> eventCallbacks;
    std::unordered_set<CallbackID> usedIds;
    CallbackID nextId = 1;
    bool debugMode = false;

public:
    // 启用/禁用调试模式
    void setDebugMode(bool enabled) { debugMode = enabled; }

    // 生成唯一ID
    CallbackID generateId() {
        while (usedIds.find(nextId) != usedIds.end()) { nextId++; }
        usedIds.insert(nextId);
        return nextId++;
    }

    // 按类型注册回调（带优先级过滤）
    CallbackID addEventCallback(EventType type, EventCallback callback, const std::string& name = "",
        EventPriority minPriority = EventPriority::Low) {
        CallbackID id = generateId();
        eventCallbacks[type].emplace_back(callback, id, name, minPriority);

        if (debugMode) {
            std::cout << "[EventManager] Registered callback '" << name << "' (ID: " << id << ") for event type "
                      << static_cast<int>(type) << std::endl;
        }
        return id;
    }

    // 为所有事件类型注册回调
    CallbackID addEventCallback(
        EventCallback callback, const std::string& name = "", EventPriority minPriority = EventPriority::Low) {
        CallbackID id = generateId();

        // 为所有事件类型注册
        eventCallbacks[EventType::TemperatureChanged].emplace_back(callback, id, name, minPriority);
        eventCallbacks[EventType::ModeChanged].emplace_back(callback, id, name, minPriority);
        eventCallbacks[EventType::SystemStateChanged].emplace_back(callback, id, name, minPriority);
        eventCallbacks[EventType::ErrorOccurred].emplace_back(callback, id, name, minPriority);

        if (debugMode) {
            std::cout << "[EventManager] Registered universal callback '" << name << "' (ID: " << id
                      << ") for all event types" << std::endl;
        }
        return id;
    }

    // 移除特定回调
    bool removeEventCallback(CallbackID id) {
        for (auto& pair : eventCallbacks) {
            auto& callbacks = pair.second;
            auto it = std::find_if(callbacks.begin(), callbacks.end(),
                [id](const EventCallbackWrapper& wrapper) { return wrapper.id == id; });

            if (it != callbacks.end()) {
                if (debugMode) {
                    std::cout << "[EventManager] Removed callback '" << it->name << "' (ID: " << id << ")" << std::endl;
                }
                callbacks.erase(it);
                usedIds.erase(id);
                return true;
            }
        }
        return false;
    }

    // 启用/禁用特定回调
    bool setCallbackEnabled(CallbackID id, bool enabled) {
        for (auto& pair : eventCallbacks) {
            for (auto& wrapper : pair.second) {
                if (wrapper.id == id) {
                    wrapper.enabled = enabled;
                    if (debugMode) {
                        std::cout << "[EventManager] " << (enabled ? "Enabled" : "Disabled") << " callback '"
                                  << wrapper.name << "' (ID: " << id << ")" << std::endl;
                    }
                    return true;
                }
            }
        }
        return false;
    }

    // 移除所有回调
    void removeAllCallbacks() {
        eventCallbacks.clear();
        usedIds.clear();
        nextId = 1;
        if (debugMode) { std::cout << "[EventManager] Removed all callbacks" << std::endl; }
    }

    // 移除特定类型的所有回调
    void removeCallbacks(EventType type) {
        for (const auto& wrapper : eventCallbacks[type]) { usedIds.erase(wrapper.id); }
        eventCallbacks[type].clear();
        if (debugMode) {
            std::cout << "[EventManager] Removed all callbacks for event type " << static_cast<int>(type) << std::endl;
        }
    }

    // 通知事件（优化版本，支持优先级过滤）
    void notifyEvent(const EventData& event) {
        std::cout << "Notifying event: type=" << static_cast<int>(event.type) << ", message=" << event.message
                  << ", value=" << event.value << ", priority=" << static_cast<int>(event.priority)
                  << ", source=" << event.source << std::endl;

        auto it = eventCallbacks.find(event.type);
        if (it != eventCallbacks.end()) {
            for (const auto& wrapper : it->second) {
                // 检查回调是否启用且优先级足够
                if (wrapper.enabled && event.priority >= wrapper.minPriority) {
                    try {
                        wrapper.callback(event);
                    } catch (const std::exception& e) {
                        std::cerr << "[EventManager] Exception in callback '" << wrapper.name << "': " << e.what()
                                  << std::endl;
                    }
                }
            }
        }
    }

    // 获取统计信息
    void printStatistics() const {
        std::cout << "\n=== Event Manager Statistics ===" << std::endl;
        std::cout << "Total registered callbacks: " << usedIds.size() << std::endl;

        for (const auto& pair : eventCallbacks) {
            std::cout << "Event type " << static_cast<int>(pair.first) << ": " << pair.second.size() << " callbacks"
                      << std::endl;
        }
        std::cout << "=================================" << std::endl;
    }
};

// 全局事件管理器实例
EventManager globalEventManager;

// 兼容性函数（向后兼容）
void addEventCallback(EventType type, EventCallback callback) { globalEventManager.addEventCallback(type, callback); }

void addEventCallback(EventCallback callback) { globalEventManager.addEventCallback(callback); }

void removeEventCallbacks() { globalEventManager.removeAllCallbacks(); }

void removeEventCallbacks(EventType type) { globalEventManager.removeCallbacks(type); }

void notifyEvent(const EventData& event) { globalEventManager.notifyEvent(event); }
void handleEvent(const EventData& event) {
    switch (event.type) {
        case EventType::TemperatureChanged:
            std::cout << "[Temperature Handler] Temperature changed to " << event.value << "°C - " << event.message
                      << std::endl;
            break;

        case EventType::ModeChanged:
            std::cout << "[Mode Handler] Mode changed to " << static_cast<int>(event.value) << " - " << event.message
                      << std::endl;
            break;

        case EventType::SystemStateChanged:
            std::cout << "[System State Handler] System state changed - " << event.message << std::endl;
            break;

        case EventType::ErrorOccurred:
            std::cout << "[Error Handler] Error occurred: " << event.message << " (Code: " << event.value << ")"
                      << std::endl;
            break;
    }
}

// 简单的事件处理器类示例
class TemperatureMonitor {
public:
    void onTemperatureChanged(const EventData& event) {
        if (event.type == EventType::TemperatureChanged) {
            std::cout << "[TemperatureMonitor] Alert: Temperature " << event.value << "°C detected!" << std::endl;
            if (event.value > 30.0) {
                std::cout << "[TemperatureMonitor] WARNING: High temperature!" << std::endl;
            } else if (event.value < 10.0) {
                std::cout << "[TemperatureMonitor] WARNING: Low temperature!" << std::endl;
            }
        }
    }
};

// 系统日志记录器
class SystemLogger {
public:
    void logEvent(const EventData& event) {
        std::cout << "[SystemLogger] Logging event: " << event.message << std::endl;
    }
};
int main(int argc, char** argv) {
    std::cout << "=== Advanced C++ Event-Driven System Demo ===" << std::endl;

    // 启用调试模式
    globalEventManager.setDebugMode(true);

    // 创建事件处理器实例
    TemperatureMonitor tempMonitor;
    SystemLogger logger;

    // 注册事件回调函数（使用新的EventManager API）
    std::cout << "\n1. Registering event handlers with advanced features..." << std::endl;

    // 按事件类型注册专用处理器（带名称和优先级）
    CallbackID tempHandlerId =
        globalEventManager.addEventCallback(EventType::TemperatureChanged, handleEvent, "TempHandler");
    CallbackID modeHandlerId = globalEventManager.addEventCallback(EventType::ModeChanged, handleEvent, "ModeHandler");
    CallbackID stateHandlerId =
        globalEventManager.addEventCallback(EventType::SystemStateChanged, handleEvent, "StateHandler");
    CallbackID errorHandlerId =
        globalEventManager.addEventCallback(EventType::ErrorOccurred, handleEvent, "ErrorHandler");

    // 注册温度监控器（只处理高优先级以上的温度事件）
    CallbackID tempMonitorId = globalEventManager.addEventCallback(
        EventType::TemperatureChanged,
        [&tempMonitor](const EventData& event) { tempMonitor.onTemperatureChanged(event); }, "TemperatureMonitor",
        EventPriority::Normal);

    // 注册系统日志记录器（处理所有事件类型，只记录高优先级事件）
    CallbackID loggerId = globalEventManager.addEventCallback(
        [&logger](const EventData& event) { logger.logEvent(event); }, "SystemLogger", EventPriority::High);

    // 注册一个关键错误处理器（只处理关键优先级的事件）
    CallbackID criticalErrorHandlerId = globalEventManager.addEventCallback(
        EventType::ErrorOccurred,
        [](const EventData& event) {
            std::cout << "[Critical Error Handler] CRITICAL: " << event.message << std::endl;
        },
        "CriticalErrorHandler", EventPriority::Critical);

    // 打印统计信息
    globalEventManager.printStatistics();

    // 模拟不同优先级的事件触发
    std::cout << "\n2. Simulating events with different priorities..." << std::endl;

    // 低优先级温度事件（不会被High优先级的Logger处理）
    std::cout << "\n--- Low Priority Events ---" << std::endl;
    notifyEvent(
        EventData(EventType::TemperatureChanged, "Low priority sensor reading", 22.0, EventPriority::Low, "SensorA"));

    // 普通优先级事件
    std::cout << "\n--- Normal Priority Events ---" << std::endl;
    notifyEvent(
        EventData(EventType::TemperatureChanged, "Normal sensor reading", 25.5, EventPriority::Normal, "SensorB"));
    notifyEvent(EventData(EventType::ModeChanged, "Mode switch", 1.0, EventPriority::Normal, "ModeController"));

    // 高优先级事件
    std::cout << "\n--- High Priority Events ---" << std::endl;
    notifyEvent(
        EventData(EventType::TemperatureChanged, "High temperature alert", 35.2, EventPriority::High, "SensorC"));
    notifyEvent(
        EventData(EventType::SystemStateChanged, "System maintenance", 0.0, EventPriority::High, "SystemManager"));

    // 关键优先级事件
    std::cout << "\n--- Critical Priority Events ---" << std::endl;
    notifyEvent(
        EventData(EventType::ErrorOccurred, "Critical system failure", 500.0, EventPriority::Critical, "SystemCore"));

    // 演示回调管理功能
    std::cout << "\n3. Testing callback management features..." << std::endl;

    // 禁用温度监控器
    std::cout << "\n--- Disabling TemperatureMonitor callback ---" << std::endl;
    globalEventManager.setCallbackEnabled(tempMonitorId, false);
    notifyEvent(
        EventData(EventType::TemperatureChanged, "Temperature after disable", 28.0, EventPriority::Normal, "SensorD"));

    // 重新启用温度监控器
    std::cout << "\n--- Re-enabling TemperatureMonitor callback ---" << std::endl;
    globalEventManager.setCallbackEnabled(tempMonitorId, true);
    notifyEvent(EventData(
        EventType::TemperatureChanged, "Temperature after re-enable", 29.0, EventPriority::Normal, "SensorE"));

    // 移除特定回调
    std::cout << "\n--- Removing specific callback ---" << std::endl;
    bool removed = globalEventManager.removeEventCallback(errorHandlerId);
    std::cout << "ErrorHandler removed: " << (removed ? "Yes" : "No") << std::endl;
    notifyEvent(EventData(EventType::ErrorOccurred, "Error after removal", 404.0, EventPriority::Normal, "Network"));

    // 最终统计信息
    std::cout << "\n4. Final statistics..." << std::endl;
    globalEventManager.printStatistics();

    // 清理所有回调
    std::cout << "\n5. Cleaning up all callbacks..." << std::endl;
    globalEventManager.removeAllCallbacks();
    std::cout << "All callbacks removed!" << std::endl;

    // 验证清理效果
    std::cout << "\n6. Testing after cleanup (should only see notification):" << std::endl;
    notifyEvent(EventData(EventType::TemperatureChanged, "Final test", 20.0, EventPriority::Normal, "Test"));

    std::cout << "\n=== Advanced Event-Driven System Demo Complete ===" << std::endl;
    return 0;
}
