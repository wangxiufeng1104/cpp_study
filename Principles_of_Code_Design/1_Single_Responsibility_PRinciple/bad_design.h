#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>

namespace bad_design {

/**
 * 坏设计示例：SensorDevice类违反单一职责原则
 * 
 * 这个类承担了多个不同的职责：
 * 1. 传感器数据读取
 * 2. MQTT数据上报
 * 3. 日志记录
 * 
 * 问题：
 * - 一个类有多个引起变化的原因
 * - 修改MQTT协议需要修改这个类
 * - 更换传感器类型需要修改这个类
 * - 修改日志格式需要修改这个类
 * - 难以独立测试各个功能
 * - 代码复用性差
 */
class SensorDevice {
private:
    float temperature_;
    float humidity_;
    bool mqttConnected_;
    std::ofstream logFile_;
    std::string deviceId_;

public:
    explicit SensorDevice(const std::string& deviceId = "sensor_001")
        : temperature_(0.0f), humidity_(0.0f), mqttConnected_(false), deviceId_(deviceId) {
        
        // 初始化日志文件
        logFile_.open("sensor_device.log", std::ios::app);
        logInfo("SensorDevice initialized: " + deviceId_);
    }
    
    ~SensorDevice() {
        if (logFile_.is_open()) {
            logInfo("SensorDevice shutdown: " + deviceId_);
            logFile_.close();
        }
    }
    
    // ============ 职责1: 传感器数据读取 ============
    
    float readTemperature() {
        // 模拟读取温度传感器（DHT11）
        temperature_ = 20.0f + (rand() % 100) / 10.0f;
        logInfo("Temperature read: " + std::to_string(temperature_) + "°C");
        return temperature_;
    }
    
    float readHumidity() {
        // 模拟读取湿度传感器（DHT11）
        humidity_ = 40.0f + (rand() % 400) / 10.0f;
        logInfo("Humidity read: " + std::to_string(humidity_) + "%");
        return humidity_;
    }
    
    // ============ 职责2: MQTT数据上报 ============
    
    bool connectMQTT() {
        // 模拟MQTT连接
        mqttConnected_ = true;
        logInfo("MQTT connected to broker");
        return mqttConnected_;
    }
    
    bool publishData(float temp, float humidity) {
        if (!mqttConnected_) {
            logError("MQTT not connected, cannot publish data");
            return false;
        }
        
        // 模拟MQTT发布数据
        std::string topic = "sensors/" + deviceId_ + "/data";
        std::string payload = "{\"device\":\"" + deviceId_ + 
                          "\",\"temp\":" + std::to_string(temp) + 
                          ",\"humidity\":" + std::to_string(humidity) + "}";
        
        logInfo("MQTT published to [" + topic + "]: " + payload);
        return true;
    }
    
    void disconnectMQTT() {
        mqttConnected_ = false;
        logInfo("MQTT disconnected from broker");
    }
    
    // ============ 职责3: 日志记录 ============
    
    void logInfo(const std::string& message) {
        logMessage("[INFO]", message);
    }
    
    void logError(const std::string& message) {
        logMessage("[ERROR]", message);
    }
    
    void logWarning(const std::string& message) {
        logMessage("[WARNING]", message);
    }
    
private:
    void logMessage(const std::string& level, const std::string& message) {
        std::cout << level << " " << message << std::endl;
        if (logFile_.is_open()) {
            logFile_ << level << " " << message << std::endl;
            logFile_.flush();
        }
    }
    
public:
    // ============ 业务逻辑：协调所有功能 ============
    
    void runControlLoop(int iterations = 5) {
        logInfo("Starting control loop with " + std::to_string(iterations) + " iterations");
        
        if (!connectMQTT()) {
            logError("Failed to connect MQTT broker");
            return;
        }
        
        for (int i = 0; i < iterations; ++i) {
            logInfo("=== Iteration " + std::to_string(i + 1) + " ===");
            
            // 读取传感器数据
            float temp = readTemperature();
            float hum = readHumidity();
            
            // 发布数据到MQTT
            if (!publishData(temp, hum)) {
                logError("Failed to publish data at iteration " + std::to_string(i + 1));
            }
            
            // 模拟设备休眠1秒
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        disconnectMQTT();
        logInfo("Control loop completed");
    }
    
    // 获取设备ID
    std::string getDeviceId() const {
        return deviceId_;
    }
    
    // 获取最后一次读取的传感器数据
    struct SensorData {
        float temperature;
        float humidity;
    };
    
    SensorData getLastSensorData() const {
        return {temperature_, humidity_};
    }
};

} // namespace bad_design
