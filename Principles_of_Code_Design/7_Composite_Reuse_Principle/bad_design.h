#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <memory>

namespace bad_design {

/**
 * 坏设计示例：使用继承进行复用（违反组合复用原则）
 * 
 * 问题：通过继承来获得能力，导致紧耦合、灵活性差
 * 原则：组合优于继承（Favor Composition over Inheritance）
 */

// 网络能力基类
class NetworkCapability {
protected:
    std::string networkType_;
    
public:
    NetworkCapability(const std::string& type) : networkType_(type) {
        std::cout << "[Bad Design] NetworkCapability created: " << type << std::endl;
    }
    
    virtual ~NetworkCapability() = default;
    
    void connect() {
        std::cout << "[Bad Design] Connecting to " << networkType_ << " network" << std::endl;
    }
    
    void sendData(const std::string& data) {
        std::cout << "[Bad Design] Sending data via " << networkType_ << ": " << data << std::endl;
    }
};

// 存储能力基类
class StorageCapability {
protected:
    std::string storageType_;
    std::string filePath_;
    
public:
    StorageCapability(const std::string& type, const std::string& path) 
        : storageType_(type), filePath_(path) {
        std::cout << "[Bad Design] StorageCapability created: " << type << std::endl;
    }
    
    virtual ~StorageCapability() = default;
    
    void write(const std::string& data) {
        std::cout << "[Bad Design] Writing to " << storageType_ << " (" << filePath_ << "): " << data << std::endl;
    }
    
    std::string read() {
        std::cout << "[Bad Design] Reading from " << storageType_ << " (" << filePath_ << ")" << std::endl;
        return "data from storage";
    }
};

// 传感器能力基类
class SensorCapability {
protected:
    std::string sensorType_;
    double value_;
    
public:
    SensorCapability(const std::string& type) : sensorType_(type), value_(0.0) {
        std::cout << "[Bad Design] SensorCapability created: " << type << std::endl;
    }
    
    virtual ~SensorCapability() = default;
    
    void setValue(double value) {
        value_ = value;
    }
    
    double getValue() const {
        return value_;
    }
    
    std::string readSensor() {
        std::cout << "[Bad Design] Reading " << sensorType_ << " sensor: " << value_ << std::endl;
        return std::to_string(value_);
    }
};

// 智能设备：使用多重继承获得所有能力
// 问题1：多重继承导致类爆炸
// 问题2：继承在编译时确定，运行时无法改变
// 问题3：违反单一职责原则
// 问题4：紧耦合，难以修改
class SmartDevice : public NetworkCapability, 
                  public StorageCapability, 
                  public SensorCapability {
private:
    std::string deviceId_;
    
public:
    SmartDevice(const std::string& deviceId, 
               const std::string& networkType,
               const std::string& storageType,
               const std::string& storagePath,
               const std::string& sensorType)
        : NetworkCapability(networkType),
          StorageCapability(storageType, storagePath),
          SensorCapability(sensorType),
          deviceId_(deviceId) {
        std::cout << "[Bad Design] SmartDevice created: " << deviceId_ << std::endl;
    }
    
    // 问题：需要知道所有父类的接口
    void sendData() {
        connect();
        NetworkCapability::sendData("{\"device\":\"" + deviceId_ + "\",\"data\":\"" + readSensor() + "\"}");
    }
    
    void saveData() {
        std::string data = readSensor();
        write(data);
    }
    
    std::string getDeviceId() const { return deviceId_; }
};

// 工业设备：使用多重继承获得部分能力
// 问题：不同设备继承不同的能力，难以统一处理
class IndustrialDevice : public NetworkCapability, 
                       public SensorCapability {
private:
    std::string deviceId_;
    
public:
    IndustrialDevice(const std::string& deviceId,
                   const std::string& networkType,
                   const std::string& sensorType)
        : NetworkCapability(networkType),
          SensorCapability(sensorType),
          deviceId_(deviceId) {
        std::cout << "[Bad Design] IndustrialDevice created: " << deviceId_ << std::endl;
    }
    
    void sendData() {
        connect();
        NetworkCapability::sendData("{\"device\":\"" + deviceId_ + "\",\"data\":\"" + readSensor() + "\"}");
    }
    
    std::string getDeviceId() const { return deviceId_; }
};

// IoT设备：使用多重继承获得部分能力
// 问题：如果需要改变能力组合，必须创建新类
class IoTDevice : public StorageCapability,
                  public SensorCapability {
private:
    std::string deviceId_;
    
public:
    IoTDevice(const std::string& deviceId,
             const std::string& storageType,
             const std::string& storagePath,
             const std::string& sensorType)
        : StorageCapability(storageType, storagePath),
          SensorCapability(sensorType),
          deviceId_(deviceId) {
        std::cout << "[Bad Design] IoTDevice created: " << deviceId_ << std::endl;
    }
    
    void saveData() {
        std::string data = readSensor();
        write(data);
    }
    
    std::string getDeviceId() const { return deviceId_; }
};

// 问题：如果需要创建一个具有网络+存储能力的设备
// 必须创建新的类，无法复用现有类
class NetworkStorageDevice : public NetworkCapability,
                            public StorageCapability {
private:
    std::string deviceId_;
    
public:
    NetworkStorageDevice(const std::string& deviceId,
                      const std::string& networkType,
                      const std::string& storageType,
                      const std::string& storagePath)
        : NetworkCapability(networkType),
          StorageCapability(storageType, storagePath),
          deviceId_(deviceId) {
        std::cout << "[Bad Design] NetworkStorageDevice created: " << deviceId_ << std::endl;
    }
    
    void transferData() {
        connect();
        std::string data = read();
        NetworkCapability::sendData(data);
    }
    
    std::string getDeviceId() const { return deviceId_; }
};

} // namespace bad_design
