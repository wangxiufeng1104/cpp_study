#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <memory>

namespace good_design {

/**
 * 好设计示例：遵循迪米特法则（Law of Demeter）
 * 
 * 优势：只与直接朋友通信，减少耦合，符合最少知识原则
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
        std::cout << "[Good Design] Sensor created: " << id_ << std::endl;
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
        std::cout << "[Good Design] Network created: " << serverUrl_ << std::endl;
    }
    
    bool connect() {
        isConnected_ = true;
        std::cout << "[Good Design] Network connected to: " << serverUrl_ << std::endl;
        return true;
    }
    
    bool send(const std::string& data) {
        if (!isConnected_) {
            std::cout << "[Good Design] Network not connected" << std::endl;
            return false;
        }
        std::cout << "[Good Design] Network sending: " << data << std::endl;
        return true;
    }
    
    void disconnect() {
        isConnected_ = false;
        std::cout << "[Good Design] Network disconnected" << std::endl;
    }
};

// 设备控制器类
// 遵循迪米特法则：提供高层接口，隐藏内部实现
class DeviceController {
private:
    std::string deviceId_;
    std::vector<std::unique_ptr<Sensor>> sensors_;
    std::unique_ptr<Network> network_;
    
public:
    DeviceController(const std::string& deviceId, const std::string& serverUrl)
        : deviceId_(deviceId), 
          network_(std::make_unique<Network>(serverUrl)) {
        std::cout << "[Good Design] DeviceController created: " << deviceId_ << std::endl;
        network_->connect();
    }
    
    void addSensor(const std::string& id, const std::string& type) {
        sensors_.push_back(std::make_unique<Sensor>(id, type));
    }
    
    // 遵循迪米特法则：提供高层接口，不暴露内部对象
    bool reportSensorData(size_t sensorIndex) {
        if (sensorIndex >= sensors_.size()) {
            std::cout << "[Good Design] Invalid sensor index" << std::endl;
            return false;
        }
        
        std::string data = sensors_[sensorIndex]->getData();
        return network_->send(data);
    }
    
    // 遵循迪米特法则：提供高层接口，不暴露内部对象
    bool reportAllSensorData() {
        std::cout << "[Good Design] Reporting all sensor data..." << std::endl;
        
        bool success = true;
        for (const auto& sensor : sensors_) {
            std::string data = sensor->getData();
            if (!network_->send(data)) {
                success = false;
            }
        }
        return success;
    }
    
    // 遵循迪米特法则：提供高层接口，不暴露内部对象
    bool getSensorValue(size_t sensorIndex, double& value) {
        if (sensorIndex >= sensors_.size()) {
            std::cout << "[Good Design] Invalid sensor index" << std::endl;
            return false;
        }
        
        value = sensors_[sensorIndex]->getValue();
        return true;
    }
    
    // 遵循迪米特法则：提供高层接口，不暴露内部对象
    bool sendFormattedData() {
        std::cout << "[Good Design] Sending formatted data..." << std::endl;
        
        for (const auto& sensor : sensors_) {
            std::string data = "{\"device\":\"" + deviceId_ + 
                             "\",\"sensor\":\"" + sensor->getId() + 
                             "\",\"type\":\"" + sensor->getType() + 
                             "\",\"value\":" + std::to_string(sensor->getValue()) + "}";
            
            if (!network_->send(data)) {
                return false;
            }
        }
        return true;
    }
    
    // 遵循迪米特法则：提供高层接口，不暴露内部对象
    size_t getSensorCount() const {
        return sensors_.size();
    }
    
    std::string getDeviceId() const {
        return deviceId_;
    }
    
    // 遵循迪米特法则：设置传感器值的高层接口
    bool setSensorValue(size_t sensorIndex, double value) {
        if (sensorIndex >= sensors_.size()) {
            std::cout << "[Good Design] Invalid sensor index" << std::endl;
            return false;
        }
        
        sensors_[sensorIndex]->setValue(value);
        return true;
    }
};

// 监控系统类
// 遵循迪米特法则：只与直接朋友DeviceController通信
class Monitor {
private:
    std::string monitorId_;
    DeviceController* controller_;
    
public:
    Monitor(const std::string& monitorId, DeviceController* controller)
        : monitorId_(monitorId), controller_(controller) {
        std::cout << "[Good Design] Monitor created: " << monitorId_ << std::endl;
    }
    
    // 遵循迪米特法则：只与直接朋友DeviceController通信
    void reportSingleSensorData(size_t sensorIndex) {
        std::cout << "[Good Design] Monitor reporting single sensor data..." << std::endl;
        
        // 优势：只与直接朋友通信，不访问内部对象
        controller_->reportSensorData(sensorIndex);
    }
    
    // 遵循迪米特法则：只与直接朋友DeviceController通信
    void reportAllSensorData() {
        std::cout << "[Good Design] Monitor reporting all sensor data..." << std::endl;
        
        // 优势：只与直接朋友通信，不访问内部对象
        controller_->reportAllSensorData();
    }
    
    // 遵循迪米特法则：只与直接朋友DeviceController通信
    void reportSpecificSensorValue(size_t sensorIndex) {
        std::cout << "[Good Design] Monitor reporting specific sensor value..." << std::endl;
        
        // 优势：通过高层接口获取值，不直接访问传感器
        double value = 0.0;
        if (controller_->getSensorValue(sensorIndex, value)) {
            std::cout << "[Good Design] Sensor value: " << value << std::endl;
            
            // 通过DeviceController发送数据
            controller_->reportSensorData(sensorIndex);
        }
    }
    
    // 遵循迪米特法则：只与直接朋友DeviceController通信
    void processDeviceData() {
        std::cout << "[Good Design] Monitor processing device data..." << std::endl;
        
        // 优势：Monitor不需要了解内部实现细节
        controller_->sendFormattedData();
    }
    
    // 遵循迪米特法则：只与直接朋友DeviceController通信
    void updateAndReportSensor(size_t sensorIndex, double newValue) {
        std::cout << "[Good Design] Monitor updating and reporting sensor..." << std::endl;
        
        // 优势：通过高层接口更新和报告
        if (controller_->setSensorValue(sensorIndex, newValue)) {
            controller_->reportSensorData(sensorIndex);
        }
    }
    
    // 遵循迪米特法则：只与直接朋友DeviceController通信
    void monitorAllSensors() {
        std::cout << "[Good Design] Monitor monitoring all sensors..." << std::endl;
        
        size_t sensorCount = controller_->getSensorCount();
        std::cout << "[Good Design] Device has " << sensorCount << " sensors" << std::endl;
        
        // 只调用高层接口，不访问内部对象
        controller_->reportAllSensorData();
    }
};

} // namespace good_design
