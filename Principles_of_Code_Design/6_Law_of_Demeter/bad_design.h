#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <memory>

namespace bad_design {

/**
 * 坏设计示例：违反迪米特法则（Law of Demeter）
 * 
 * 问题：类直接访问"陌生人"对象（间接依赖的对象）
 * 导致：高耦合、难以维护、违反最少知识原则
 */

// 传感器类
class Sensor {
private:
    std::string id_;
    std::string type_;
    double value_;
    
public:
    Sensor(const std::string& id, const std::string& type)
        : id_(id), type_(type), value_(0.0) {
        std::cout << "[Bad Design] Sensor created: " << id_ << std::endl;
    }
    
    void setValue(double value) {
        value_ = value;
    }
    
    double getValue() const {
        return value_;
    }
    
    std::string getId() const {
        return id_;
    }
    
    std::string getType() const {
        return type_;
    }
    
    std::string getData() const {
        return "{\"id\":\"" + id_ + "\",\"type\":\"" + type_ + "\",\"value\":" + 
               std::to_string(value_) + "}";
    }
};

// 网络类
class Network {
private:
    std::string serverUrl_;
    bool isConnected_;
    
public:
    Network(const std::string& serverUrl)
        : serverUrl_(serverUrl), isConnected_(false) {
        std::cout << "[Bad Design] Network created: " << serverUrl_ << std::endl;
    }
    
    bool connect() {
        isConnected_ = true;
        std::cout << "[Bad Design] Network connected to: " << serverUrl_ << std::endl;
        return true;
    }
    
    bool send(const std::string& data) {
        if (!isConnected_) {
            std::cout << "[Bad Design] Network not connected" << std::endl;
            return false;
        }
        std::cout << "[Bad Design] Network sending: " << data << std::endl;
        return true;
    }
    
    void disconnect() {
        isConnected_ = false;
        std::cout << "[Bad Design] Network disconnected" << std::endl;
    }
};

// 设备控制器类
class DeviceController {
private:
    std::string deviceId_;
    std::vector<std::unique_ptr<Sensor>> sensors_;
    std::unique_ptr<Network> network_;
    
public:
    DeviceController(const std::string& deviceId, const std::string& serverUrl)
        : deviceId_(deviceId), 
          network_(std::make_unique<Network>(serverUrl)) {
        std::cout << "[Bad Design] DeviceController created: " << deviceId_ << std::endl;
        network_->connect();
    }
    
    void addSensor(const std::string& id, const std::string& type) {
        sensors_.push_back(std::make_unique<Sensor>(id, type));
    }
    
    // 违反迪米特法则：暴露内部传感器对象
    Sensor* getSensor(size_t index) {
        if (index < sensors_.size()) {
            return sensors_[index].get();
        }
        return nullptr;
    }
    
    // 违反迪米特法则：暴露内部网络对象
    Network* getNetwork() {
        return network_.get();
    }
    
    // 违反迪米特法则：返回传感器数量（暴露内部状态）
    size_t getSensorCount() const {
        return sensors_.size();
    }
    
    std::string getDeviceId() const {
        return deviceId_;
    }
};

// 监控系统类
// 问题：直接访问"陌生人"对象（DeviceController内部的Sensor和Network）
class Monitor {
private:
    std::string monitorId_;
    DeviceController* controller_;
    
public:
    Monitor(const std::string& monitorId, DeviceController* controller)
        : monitorId_(monitorId), controller_(controller) {
        std::cout << "[Bad Design] Monitor created: " << monitorId_ << std::endl;
    }
    
    // 违反迪米特法则：访问"陌生人"对象
    void reportSingleSensorData(size_t sensorIndex) {
        std::cout << "[Bad Design] Monitor reporting single sensor data..." << std::endl;
        
        // 问题1：获取传感器（陌生人）
        Sensor* sensor = controller_->getSensor(sensorIndex);
        if (sensor) {
            // 问题2：访问传感器的方法（与陌生人通信）
            std::string data = sensor->getData();
            
            // 问题3：获取网络（陌生人）
            Network* network = controller_->getNetwork();
            if (network) {
                // 问题4：发送数据（与陌生人通信）
                network->send(data);
            }
        }
    }
    
    // 违反迪米特法则：访问"陌生人"对象
    void reportAllSensorData() {
        std::cout << "[Bad Design] Monitor reporting all sensor data..." << std::endl;
        
        // 问题1：获取传感器数量（暴露内部状态）
        size_t count = controller_->getSensorCount();
        
        // 问题2：遍历并访问传感器（陌生人）
        for (size_t i = 0; i < count; ++i) {
            Sensor* sensor = controller_->getSensor(i);
            if (sensor) {
                // 问题3：访问传感器的方法（与陌生人通信）
                std::string data = sensor->getData();
                
                // 问题4：获取网络（陌生人）
                Network* network = controller_->getNetwork();
                if (network) {
                    // 问题5：发送数据（与陌生人通信）
                    network->send(data);
                }
            }
        }
    }
    
    // 违反迪米特法则：深入访问"陌生人"对象
    void reportSpecificSensorValue(size_t sensorIndex) {
        std::cout << "[Bad Design] Monitor reporting specific sensor value..." << std::endl;
        
        // 问题：深层访问
        Sensor* sensor = controller_->getSensor(sensorIndex);
        if (sensor) {
            // 直接访问传感器的内部状态
            double value = sensor->getValue();
            std::cout << "[Bad Design] Sensor value: " << value << std::endl;
            
            // 获取网络并使用
            Network* network = controller_->getNetwork();
            if (network) {
                std::string msg = "{\"value\":" + std::to_string(value) + "}";
                network->send(msg);
            }
        }
    }
    
    // 违反迪米特法则：暴露内部逻辑
    void processDeviceData() {
        std::cout << "[Bad Design] Monitor processing device data..." << std::endl;
        
        // 问题：Monitor需要了解DeviceController的内部结构
        size_t sensorCount = controller_->getSensorCount();
        Network* network = controller_->getNetwork();
        
        for (size_t i = 0; i < sensorCount; ++i) {
            Sensor* sensor = controller_->getSensor(i);
            if (sensor) {
                std::string id = sensor->getId();
                std::string type = sensor->getType();
                double value = sensor->getValue();
                
                // Monitor需要理解如何构造数据格式
                std::string data = "{\"device\":\"" + controller_->getDeviceId() + 
                                 "\",\"sensor\":\"" + id + 
                                 "\",\"type\":\"" + type + 
                                 "\",\"value\":" + std::to_string(value) + "}";
                
                network->send(data);
            }
        }
    }
};

} // namespace bad_design
