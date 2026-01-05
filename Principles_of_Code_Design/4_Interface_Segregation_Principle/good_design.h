#pragma once

#include <string>
#include <iostream>
#include <vector>

namespace good_design {

/**
 * 好设计示例：遵循接口隔离原则
 * 
 * 优势：将臃肿的接口拆分为多个细分的专用接口
 * 每个客户端只需实现真正需要的方法
 */

// 细分的专用接口

// 传感器接口：只包含传感器相关方法
class ISensor {
public:
    virtual ~ISensor() = default;
    virtual double readValue() = 0;
};

// 执行器接口：只包含执行器相关方法
class IActuator {
public:
    virtual ~IActuator() = default;
    virtual void execute(const std::string& command) = 0;
};

// 显示器接口：只包含显示器相关方法
class IDisplay {
public:
    virtual ~IDisplay() = default;
    virtual void show(const std::string& message) = 0;
};

// 网络接口：只包含网络相关方法
class INetwork {
public:
    virtual ~INetwork() = default;
    virtual bool connect(const std::string& address, int port) = 0;
    virtual bool send(const std::string& data) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
};

// 存储接口：只包含存储相关方法
class IStorage {
public:
    virtual ~IStorage() = default;
    virtual bool save(const std::string& data) = 0;
    virtual bool load(std::string& data) = 0;
};

// 日志接口：只包含日志相关方法
class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void info(const std::string& message) = 0;
    virtual void error(const std::string& error) = 0;
};

// 具体实现类

// 传感器实现：只需实现ISensor接口
class TemperatureSensor : public ISensor {
private:
    double temperature_;
    std::string sensorId_;
    
public:
    TemperatureSensor(const std::string& id) 
        : temperature_(25.0), sensorId_(id) {
        std::cout << "[Good Design] TemperatureSensor created: " << id << std::endl;
    }
    
    // 只实现需要的方法
    double readValue() override {
        std::cout << "[Good Design] TemperatureSensor reading: " << temperature_ << "°C" << std::endl;
        return temperature_;
    }
};

// 执行器实现：只需实现IActuator接口
class MotorController : public IActuator {
private:
    bool isRunning_;
    std::string motorId_;
    
public:
    MotorController(const std::string& id)
        : isRunning_(false), motorId_(id) {
        std::cout << "[Good Design] MotorController created: " << id << std::endl;
    }
    
    // 只实现需要的方法
    void execute(const std::string& command) override {
        std::cout << "[Good Design] MotorController executing: " << command << std::endl;
        if (command == "start") {
            isRunning_ = true;
            std::cout << "[Good Design] Motor started" << std::endl;
        } else if (command == "stop") {
            isRunning_ = false;
            std::cout << "[Good Design] Motor stopped" << std::endl;
        }
    }
};

// 显示器实现：只需实现IDisplay接口
class DisplayModule : public IDisplay {
private:
    std::string displayId_;
    
public:
    DisplayModule(const std::string& id)
        : displayId_(id) {
        std::cout << "[Good Design] DisplayModule created: " << id << std::endl;
    }
    
    // 只实现需要的方法
    void show(const std::string& message) override {
        std::cout << "[Good Design] DisplayModule showing: " << message << std::endl;
    }
};

// 网络实现：只需实现INetwork接口
class WifiNetwork : public INetwork {
private:
    bool connected_;
    std::string ssid_;
    
public:
    WifiNetwork(const std::string& ssid)
        : connected_(false), ssid_(ssid) {
        std::cout << "[Good Design] WifiNetwork created: " << ssid << std::endl;
    }
    
    bool connect(const std::string& address, int port) override {
        std::cout << "[Good Design] WifiNetwork connecting to " << address << ":" << port << std::endl;
        connected_ = true;
        return true;
    }
    
    bool send(const std::string& data) override {
        if (!connected_) {
            std::cout << "[Good Design] WifiNetwork not connected" << std::endl;
            return false;
        }
        std::cout << "[Good Design] WifiNetwork sent: " << data << std::endl;
        return true;
    }
    
    void disconnect() override {
        std::cout << "[Good Design] WifiNetwork disconnecting" << std::endl;
        connected_ = false;
    }
    
    bool isConnected() const override {
        return connected_;
    }
};

// 存储实现：只需实现IStorage接口
class FileStorage : public IStorage {
private:
    std::string filename_;
    std::vector<std::string> data_;
    
public:
    FileStorage(const std::string& filename)
        : filename_(filename) {
        std::cout << "[Good Design] FileStorage created: " << filename << std::endl;
    }
    
    bool save(const std::string& data) override {
        data_.push_back(data);
        std::cout << "[Good Design] FileStorage saved: " << data << std::endl;
        return true;
    }
    
    bool load(std::string& data) override {
        if (data_.empty()) {
            return false;
        }
        data = data_.back();
        std::cout << "[Good Design] FileStorage loaded: " << data << std::endl;
        return true;
    }
};

// 日志实现：只需实现ILogger接口
class ConsoleLogger : public ILogger {
private:
    std::string loggerId_;
    
public:
    ConsoleLogger(const std::string& id)
        : loggerId_(id) {
        std::cout << "[Good Design] ConsoleLogger created: " << id << std::endl;
    }
    
    void info(const std::string& message) override {
        std::cout << "[Good Design] [INFO] " << message << std::endl;
    }
    
    void error(const std::string& error) override {
        std::cout << "[Good Design] [ERROR] " << error << std::endl;
    }
};

// 复杂设备：可以组合多个接口
// 展示如何一个类实现多个接口
class SmartDevice : public ISensor, public IActuator, public IDisplay {
private:
    std::string deviceId_;
    
public:
    SmartDevice(const std::string& id)
        : deviceId_(id) {
        std::cout << "[Good Design] SmartDevice created: " << id << std::endl;
    }
    
    // 实现ISensor接口
    double readValue() override {
        std::cout << "[Good Design] SmartDevice reading sensor" << std::endl;
        return 26.5;
    }
    
    // 实现IActuator接口
    void execute(const std::string& command) override {
        std::cout << "[Good Design] SmartDevice executing: " << command << std::endl;
    }
    
    // 实现IDisplay接口
    void show(const std::string& message) override {
        std::cout << "[Good Design] SmartDevice showing: " << message << std::endl;
    }
};

// 更复杂的设备：组合更多接口
class NetworkedSmartDevice : public ISensor, public IActuator, public INetwork, public IStorage {
private:
    std::string deviceId_;
    
public:
    NetworkedSmartDevice(const std::string& id)
        : deviceId_(id) {
        std::cout << "[Good Design] NetworkedSmartDevice created: " << id << std::endl;
    }
    
    // 实现ISensor接口
    double readValue() override {
        std::cout << "[Good Design] NetworkedSmartDevice reading sensor" << std::endl;
        return 27.0;
    }
    
    // 实现IActuator接口
    void execute(const std::string& command) override {
        std::cout << "[Good Design] NetworkedSmartDevice executing: " << command << std::endl;
    }
    
    // 实现INetwork接口
    bool connect(const std::string& address, int port) override {
        std::cout << "[Good Design] NetworkedSmartDevice connecting" << std::endl;
        return true;
    }
    
    bool send(const std::string& data) override {
        std::cout << "[Good Design] NetworkedSmartDevice sent: " << data << std::endl;
        return true;
    }
    
    void disconnect() override {
        std::cout << "[Good Design] NetworkedSmartDevice disconnecting" << std::endl;
    }
    
    bool isConnected() const override {
        return true;
    }
    
    // 实现IStorage接口
    bool save(const std::string& data) override {
        std::cout << "[Good Design] NetworkedSmartDevice saved: " << data << std::endl;
        return true;
    }
    
    bool load(std::string& data) override {
        data = "loaded_data";
        std::cout << "[Good Design] NetworkedSmartDevice loaded" << std::endl;
        return true;
    }
};

} // namespace good_design
