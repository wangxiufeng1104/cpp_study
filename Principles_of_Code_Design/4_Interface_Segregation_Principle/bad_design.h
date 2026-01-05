#pragma once

#include <string>
#include <iostream>
#include <stdexcept>

namespace bad_design {

/**
 * 坏设计示例：违反接口隔离原则
 * 
 * 问题：设计了一个臃肿的接口，包含所有可能的方法
 * 导致客户端被迫实现不需要的方法
 */

// 臃肿的接口：包含所有可能的设备功能
class IDevice {
public:
    virtual ~IDevice() = default;
    
    // 传感器相关方法
    virtual double readSensor() = 0;
    
    // 执行器相关方法
    virtual void executeCommand(const std::string& command) = 0;
    
    // 显示器相关方法
    virtual void display(const std::string& message) = 0;
    
    // 网络相关方法
    virtual bool connectNetwork(const std::string& address, int port) = 0;
    virtual bool sendNetworkData(const std::string& data) = 0;
    virtual void disconnectNetwork() = 0;
    
    // 存储相关方法
    virtual bool saveData(const std::string& data) = 0;
    virtual bool loadData(std::string& data) = 0;
    
    // 日志相关方法
    virtual void logInfo(const std::string& message) = 0;
    virtual void logError(const std::string& error) = 0;
};

// 传感器类：被迫实现不需要的方法
class TemperatureSensor : public IDevice {
private:
    double temperature_;
    std::string sensorId_;
    
public:
    TemperatureSensor(const std::string& id) 
        : temperature_(25.0), sensorId_(id) {
        std::cout << "[Bad Design] TemperatureSensor created: " << id << std::endl;
    }
    
    // 真正需要的方法
    double readSensor() override {
        std::cout << "[Bad Design] TemperatureSensor reading: " << temperature_ << "°C" << std::endl;
        return temperature_;
    }
    
    // 被迫实现的空方法（违反ISP）
    void executeCommand(const std::string& command) override {
        std::cout << "[Bad Design] ⚠️ TemperatureSensor不应该实现executeCommand()!" << std::endl;
        throw std::runtime_error("Not implemented");
    }
    
    void display(const std::string& message) override {
        std::cout << "[Bad Design] ⚠️ TemperatureSensor不应该实现display()!" << std::endl;
        throw std::runtime_error("Not implemented");
    }
    
    bool connectNetwork(const std::string& address, int port) override {
        std::cout << "[Bad Design] ⚠️ TemperatureSensor不应该实现connectNetwork()!" << std::endl;
        throw std::runtime_error("Not implemented");
    }
    
    bool sendNetworkData(const std::string& data) override {
        std::cout << "[Bad Design] ⚠️ TemperatureSensor不应该实现sendNetworkData()!" << std::endl;
        throw std::runtime_error("Not implemented");
    }
    
    void disconnectNetwork() override {
        std::cout << "[Bad Design] ⚠️ TemperatureSensor不应该实现disconnectNetwork()!" << std::endl;
    }
    
    bool saveData(const std::string& data) override {
        std::cout << "[Bad Design] ⚠️ TemperatureSensor不应该实现saveData()!" << std::endl;
        return false;
    }
    
    bool loadData(std::string& data) override {
        std::cout << "[Bad Design] ⚠️ TemperatureSensor不应该实现loadData()!" << std::endl;
        return false;
    }
    
    void logInfo(const std::string& message) override {
        std::cout << "[Bad Design] ⚠️ TemperatureSensor不应该实现logInfo()!" << std::endl;
    }
    
    void logError(const std::string& error) override {
        std::cout << "[Bad Design] ⚠️ TemperatureSensor不应该实现logError()!" << std::endl;
    }
};

// 执行器类：被迫实现不需要的方法
class MotorController : public IDevice {
private:
    bool isRunning_;
    std::string motorId_;
    
public:
    MotorController(const std::string& id)
        : isRunning_(false), motorId_(id) {
        std::cout << "[Bad Design] MotorController created: " << id << std::endl;
    }
    
    // 真正需要的方法
    void executeCommand(const std::string& command) override {
        std::cout << "[Bad Design] MotorController executing: " << command << std::endl;
        if (command == "start") {
            isRunning_ = true;
            std::cout << "[Bad Design] Motor started" << std::endl;
        } else if (command == "stop") {
            isRunning_ = false;
            std::cout << "[Bad Design] Motor stopped" << std::endl;
        }
    }
    
    // 被迫实现的空方法（违反ISP）
    double readSensor() override {
        std::cout << "[Bad Design] ⚠️ MotorController不应该实现readSensor()!" << std::endl;
        throw std::runtime_error("Not implemented");
    }
    
    void display(const std::string& message) override {
        std::cout << "[Bad Design] ⚠️ MotorController不应该实现display()!" << std::endl;
        throw std::runtime_error("Not implemented");
    }
    
    bool connectNetwork(const std::string& address, int port) override {
        std::cout << "[Bad Design] ⚠️ MotorController不应该实现connectNetwork()!" << std::endl;
        throw std::runtime_error("Not implemented");
    }
    
    bool sendNetworkData(const std::string& data) override {
        std::cout << "[Bad Design] ⚠️ MotorController不应该实现sendNetworkData()!" << std::endl;
        throw std::runtime_error("Not implemented");
    }
    
    void disconnectNetwork() override {
        std::cout << "[Bad Design] ⚠️ MotorController不应该实现disconnectNetwork()!" << std::endl;
    }
    
    bool saveData(const std::string& data) override {
        std::cout << "[Bad Design] ⚠️ MotorController不应该实现saveData()!" << std::endl;
        return false;
    }
    
    bool loadData(std::string& data) override {
        std::cout << "[Bad Design] ⚠️ MotorController不应该实现loadData()!" << std::endl;
        return false;
    }
    
    void logInfo(const std::string& message) override {
        std::cout << "[Bad Design] ⚠️ MotorController不应该实现logInfo()!" << std::endl;
    }
    
    void logError(const std::string& error) override {
        std::cout << "[Bad Design] ⚠️ MotorController不应该实现logError()!" << std::endl;
    }
};

// 显示器类：被迫实现不需要的方法
class DisplayModule : public IDevice {
private:
    std::string displayId_;
    
public:
    DisplayModule(const std::string& id)
        : displayId_(id) {
        std::cout << "[Bad Design] DisplayModule created: " << id << std::endl;
    }
    
    // 真正需要的方法
    void display(const std::string& message) override {
        std::cout << "[Bad Design] DisplayModule showing: " << message << std::endl;
    }
    
    // 被迫实现的空方法（违反ISP）
    double readSensor() override {
        std::cout << "[Bad Design] ⚠️ DisplayModule不应该实现readSensor()!" << std::endl;
        throw std::runtime_error("Not implemented");
    }
    
    void executeCommand(const std::string& command) override {
        std::cout << "[Bad Design] ⚠️ DisplayModule不应该实现executeCommand()!" << std::endl;
        throw std::runtime_error("Not implemented");
    }
    
    bool connectNetwork(const std::string& address, int port) override {
        std::cout << "[Bad Design] ⚠️ DisplayModule不应该实现connectNetwork()!" << std::endl;
        throw std::runtime_error("Not implemented");
    }
    
    bool sendNetworkData(const std::string& data) override {
        std::cout << "[Bad Design] ⚠️ DisplayModule不应该实现sendNetworkData()!" << std::endl;
        throw std::runtime_error("Not implemented");
    }
    
    void disconnectNetwork() override {
        std::cout << "[Bad Design] ⚠️ DisplayModule不应该实现disconnectNetwork()!" << std::endl;
    }
    
    bool saveData(const std::string& data) override {
        std::cout << "[Bad Design] ⚠️ DisplayModule不应该实现saveData()!" << std::endl;
        return false;
    }
    
    bool loadData(std::string& data) override {
        std::cout << "[Bad Design] ⚠️ DisplayModule不应该实现loadData()!" << std::endl;
        return false;
    }
    
    void logInfo(const std::string& message) override {
        std::cout << "[Bad Design] ⚠️ DisplayModule不应该实现logInfo()!" << std::endl;
    }
    
    void logError(const std::string& error) override {
        std::cout << "[Bad Design] ⚠️ DisplayModule不应该实现logError()!" << std::endl;
    }
};

} // namespace bad_design
