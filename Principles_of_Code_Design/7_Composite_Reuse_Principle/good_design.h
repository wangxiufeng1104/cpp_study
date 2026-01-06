#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>

namespace good_design {

/**
 * 好设计示例：使用组合进行复用（遵循组合复用原则）
 * 
 * 优势：通过组合来获得能力，松耦合、灵活性高
 * 原则：组合优于继承（Favor Composition over Inheritance）
 */

// 能力接口
class ICapability {
public:
    virtual ~ICapability() = default;
    
    virtual std::string getType() const = 0;
    virtual void execute() = 0;
};

// 网络能力实现
class NetworkCapability : public ICapability {
private:
    std::string networkType_;
    
public:
    NetworkCapability(const std::string& type) : networkType_(type) {
        std::cout << "[Good Design] NetworkCapability created: " << type << std::endl;
    }
    
    std::string getType() const override {
        return "network";
    }
    
    void connect() {
        std::cout << "[Good Design] Connecting to " << networkType_ << " network" << std::endl;
    }
    
    void sendData(const std::string& data) {
        std::cout << "[Good Design] Sending data via " << networkType_ << ": " << data << std::endl;
    }
    
    void execute() override {
        connect();
        sendData("test data");
    }
};

// 存储能力实现
class StorageCapability : public ICapability {
private:
    std::string storageType_;
    std::string filePath_;
    
public:
    StorageCapability(const std::string& type, const std::string& path) 
        : storageType_(type), filePath_(path) {
        std::cout << "[Good Design] StorageCapability created: " << type << std::endl;
    }
    
    std::string getType() const override {
        return "storage";
    }
    
    void write(const std::string& data) {
        std::cout << "[Good Design] Writing to " << storageType_ << " (" << filePath_ << "): " << data << std::endl;
    }
    
    std::string read() {
        std::cout << "[Good Design] Reading from " << storageType_ << " (" << filePath_ << ")" << std::endl;
        return "data from storage";
    }
    
    void execute() override {
        write("test data");
    }
};

// 传感器能力实现
class SensorCapability : public ICapability {
private:
    std::string sensorType_;
    double value_;
    
public:
    SensorCapability(const std::string& type) : sensorType_(type), value_(0.0) {
        std::cout << "[Good Design] SensorCapability created: " << type << std::endl;
    }
    
    std::string getType() const override {
        return "sensor";
    }
    
    void setValue(double value) {
        value_ = value;
    }
    
    double getValue() const {
        return value_;
    }
    
    std::string readSensor() {
        std::cout << "[Good Design] Reading " << sensorType_ << " sensor: " << value_ << std::endl;
        return std::to_string(value_);
    }
    
    void execute() override {
        readSensor();
    }
};

// 设备基类：使用组合来获得能力
// 优势：运行时可以动态添加/移除能力
class Device {
private:
    std::string deviceId_;
    std::vector<std::unique_ptr<ICapability>> capabilities_;
    
public:
    Device(const std::string& deviceId) : deviceId_(deviceId) {
        std::cout << "[Good Design] Device created: " << deviceId_ << std::endl;
    }
    
    virtual ~Device() = default;
    
    // 优势：运行时动态添加能力
    void addCapability(std::unique_ptr<ICapability> capability) {
        capabilities_.push_back(std::move(capability));
    }
    
    // 优势：运行时动态移除能力
    void removeCapability(const std::string& type) {
        capabilities_.erase(
            std::remove_if(capabilities_.begin(), capabilities_.end(),
                [&type](const std::unique_ptr<ICapability>& cap) {
                    return cap->getType() == type;
                }),
            capabilities_.end());
    }
    
    // 优势：根据类型获取能力
    ICapability* getCapability(const std::string& type) const {
        for (const auto& cap : capabilities_) {
            if (cap->getType() == type) {
                return cap.get();
            }
        }
        return nullptr;
    }
    
    // 优势：执行特定类型的能力
    void executeCapability(const std::string& type) {
        for (const auto& cap : capabilities_) {
            if (cap->getType() == type) {
                cap->execute();
            }
        }
    }
    
    // 优势：执行所有能力
    void executeAllCapabilities() {
        for (const auto& cap : capabilities_) {
            cap->execute();
        }
    }
    
    // 优势：获取能力列表
    std::vector<std::string> getCapabilityTypes() const {
        std::vector<std::string> types;
        for (const auto& cap : capabilities_) {
            types.push_back(cap->getType());
        }
        return types;
    }
    
    std::string getDeviceId() const { return deviceId_; }
    
    virtual void performTask() {
        std::cout << "[Good Design] Device performing task..." << std::endl;
        executeAllCapabilities();
    }
};

// 智能设备：使用组合获得所有能力
class SmartDevice : public Device {
public:
    SmartDevice(const std::string& deviceId) : Device(deviceId) {
        std::cout << "[Good Design] SmartDevice created" << std::endl;
    }
    
    void performTask() override {
        std::cout << "[Good Design] SmartDevice performing comprehensive task..." << std::endl;
        executeAllCapabilities();
    }
};

// 工业设备：使用组合获得部分能力
class IndustrialDevice : public Device {
public:
    IndustrialDevice(const std::string& deviceId) : Device(deviceId) {
        std::cout << "[Good Design] IndustrialDevice created" << std::endl;
    }
    
    void performTask() override {
        std::cout << "[Good Design] IndustrialDevice performing monitoring task..." << std::endl;
        executeCapability("network");
        executeCapability("sensor");
    }
};

// IoT设备：使用组合获得部分能力
class IoTDevice : public Device {
public:
    IoTDevice(const std::string& deviceId) : Device(deviceId) {
        std::cout << "[Good Design] IoTDevice created" << std::endl;
    }
    
    void performTask() override {
        std::cout << "[Good Design] IoTDevice performing storage task..." << std::endl;
        executeCapability("storage");
        executeCapability("sensor");
    }
};

// 设备工厂：用于创建设备并添加能力
class DeviceFactory {
public:
    // 优势：通过工厂模式灵活创建设备
    static std::unique_ptr<Device> createSmartDevice(const std::string& deviceId) {
        auto device = std::make_unique<SmartDevice>(deviceId);
        device->addCapability(std::make_unique<NetworkCapability>("WiFi"));
        device->addCapability(std::make_unique<StorageCapability>("Flash", "/data/smart.txt"));
        device->addCapability(std::make_unique<SensorCapability>("Temperature"));
        return device;
    }
    
    static std::unique_ptr<Device> createIndustrialDevice(const std::string& deviceId) {
        auto device = std::make_unique<IndustrialDevice>(deviceId);
        device->addCapability(std::make_unique<NetworkCapability>("Ethernet"));
        device->addCapability(std::make_unique<SensorCapability>("Humidity"));
        return device;
    }
    
    static std::unique_ptr<Device> createIoTDevice(const std::string& deviceId) {
        auto device = std::make_unique<IoTDevice>(deviceId);
        device->addCapability(std::make_unique<StorageCapability>("SD", "/mnt/sd/iot.txt"));
        device->addCapability(std::make_unique<SensorCapability>("Motion"));
        return device;
    }
    
    // 优势：可以灵活创建任意能力组合的设备
    static std::unique_ptr<Device> createCustomDevice(
        const std::string& deviceId,
        const std::string& networkType,
        const std::string& storageType,
        const std::string& storagePath,
        const std::string& sensorType) {
        
        auto device = std::make_unique<Device>(deviceId);
        
        if (!networkType.empty()) {
            device->addCapability(std::make_unique<NetworkCapability>(networkType));
        }
        
        if (!storageType.empty()) {
            device->addCapability(std::make_unique<StorageCapability>(storageType, storagePath));
        }
        
        if (!sensorType.empty()) {
            device->addCapability(std::make_unique<SensorCapability>(sensorType));
        }
        
        return device;
    }
};

// 设备管理器：统一管理所有设备
class DeviceManager {
private:
    std::vector<std::unique_ptr<Device>> devices_;
    
public:
    void addDevice(std::unique_ptr<Device> device) {
        devices_.push_back(std::move(device));
    }
    
    void executeAllDevices() {
        std::cout << "[Good Design] Executing all devices..." << std::endl;
        for (const auto& device : devices_) {
            device->performTask();
        }
    }
    
    Device* findDevice(const std::string& deviceId) {
        for (const auto& device : devices_) {
            if (device->getDeviceId() == deviceId) {
                return device.get();
            }
        }
        return nullptr;
    }
    
    void addCapabilityToDevice(const std::string& deviceId, 
                            std::unique_ptr<ICapability> capability) {
        Device* device = findDevice(deviceId);
        if (device) {
            device->addCapability(std::move(capability));
            std::cout << "[Good Design] Added capability to device: " << deviceId << std::endl;
        }
    }
    
    void removeCapabilityFromDevice(const std::string& deviceId, 
                                const std::string& capabilityType) {
        Device* device = findDevice(deviceId);
        if (device) {
            device->removeCapability(capabilityType);
            std::cout << "[Good Design] Removed capability from device: " << deviceId << std::endl;
        }
    }
};

} // namespace good_design
