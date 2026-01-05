#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

namespace bad_design {

/**
 * 坏设计示例：SensorDevice类违反单一职责原则
 * 
 * 这个类承担了3个不同的职责：
 * 1. 传感器数据读取
 * 2. MQTT数据上报
 * 3. 日志记录
 */
class SensorDevice {
private:
    float temperature_;
    float humidity_;
    bool mqttConnected_;
    std::ofstream logFile_;

public:
    SensorDevice() : temperature_(0.0f), humidity_(0.0f), mqttConnected_(false) {
        // 初始化日志文件
        logFile_.open("sensor_device.log", std::ios::app);
        logInfo("SensorDevice initialized");
    }
    
    ~SensorDevice() {
        if (logFile_.is_open()) {
            logInfo("SensorDevice shutdown");
            logFile_.close();
        }
    }
    
    // 职责1: 传感器数据读取
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
    
    // 职责2: MQTT连接和数据上报
    bool connectMQTT() {
        // 模拟MQTT连接
        mqttConnected_ = true;
        logInfo("MQTT connected");
        return mqttConnected_;
    }
    
    bool publishData(float temp, float humidity) {
        if (!mqttConnected_) {
            logError("MQTT not connected, cannot publish");
            return false;
        }
        
        // 模拟MQTT发布数据
        std::string payload = "{\"temp\":" + std::to_string(temp) + 
                          ",\"humidity\":" + std::to_string(humidity) + "}";
        logInfo("MQTT published: " + payload);
        return true;
    }
    
    // 职责3: 日志记录
    void logInfo(const std::string& message) {
        logMessage("[INFO]", message);
    }
    
    void logError(const std::string& message) {
        logMessage("[ERROR]", message);
    }
    
private:
    void logMessage(const std::string& level, const std::string& message) {
        std::cout << level << " " << message << std::endl;
        if (logFile_.is_open()) {
            logFile_ << level << " " << message << std::endl;
        }
    }
    
public:
    // 业务逻辑：协调所有功能
    void runControlLoop(int iterations = 5) {
        logInfo("Starting control loop with " + std::to_string(iterations) + " iterations");
        
        if (!connectMQTT()) {
            logError("Failed to connect MQTT");
            return;
        }
        
        for (int i = 0; i < iterations; ++i) {
            float temp = readTemperature();
            float hum = readHumidity();
            
            if (!publishData(temp, hum)) {
                logError("Failed to publish data");
            }
            
            // 模拟设备休眠1秒
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        logInfo("Control loop completed");
    }
};

} // namespace bad_design

namespace good_design {

// 职责1: 传感器读取器 - 只负责传感器数据采集
class SensorReader {
public:
    SensorReader() = default;
    
    struct SensorData {
        float temperature;
        float humidity;
    };
    
    // 只负责读取传感器数据
    SensorData read() {
        SensorData data;
        data.temperature = 20.0f + (rand() % 100) / 10.0f;
        data.humidity = 40.0f + (rand() % 400) / 10.0f;
        return data;
    }
};

// 职责2: MQTT发布器 - 只负责数据上报
class MqttPublisher {
private:
    bool connected_;
    
public:
    MqttPublisher() : connected_(false) {}
    
    bool connect() {
        // 模拟MQTT连接
        connected_ = true;
        std::cout << "[MQTT] Connected to broker" << std::endl;
        return connected_;
    }
    
    bool publish(float temp, float humidity) {
        if (!connected_) {
            std::cout << "[MQTT] Error: Not connected" << std::endl;
            return false;
        }
        
        std::string payload = "{\"temp\":" + std::to_string(temp) + 
                          ",\"humidity\":" + std::to_string(humidity) + "}";
        std::cout << "[MQTT] Published: " << payload << std::endl;
        return true;
    }
    
    void disconnect() {
        connected_ = false;
        std::cout << "[MQTT] Disconnected" << std::endl;
    }
};

// 职责3: 设备日志器 - 只负责日志记录
class DeviceLogger {
private:
    std::ofstream logFile_;
    
public:
    DeviceLogger() {
        logFile_.open("device.log", std::ios::app);
    }
    
    ~DeviceLogger() {
        if (logFile_.is_open()) {
            logFile_.close();
        }
    }
    
    void log(const std::string& message) {
        std::cout << "[LOG] " << message << std::endl;
        if (logFile_.is_open()) {
            logFile_ << "[LOG] " << message << std::endl;
        }
    }
};

// 业务逻辑控制器 - 只负责协调各个组件
class SensorController {
private:
    SensorReader sensorReader_;
    MqttPublisher mqttPublisher_;
    DeviceLogger logger_;
    
public:
    SensorController() = default;
    
    void runControlLoop(int iterations = 5) {
        logger_.log("Starting control loop with " + std::to_string(iterations) + " iterations");
        
        if (!mqttPublisher_.connect()) {
            logger_.log("Failed to connect MQTT broker");
            return;
        }
        
        for (int i = 0; i < iterations; ++i) {
            // 使用传感器读取器
            auto data = sensorReader_.read();
            logger_.log("Read - Temp: " + std::to_string(data.temperature) + 
                      "°C, Humidity: " + std::to_string(data.humidity) + "%");
            
            // 使用MQTT发布器
            if (!mqttPublisher_.publish(data.temperature, data.humidity)) {
                logger_.log("Failed to publish data");
            }
            
            // 模拟设备休眠
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        mqttPublisher_.disconnect();
        logger_.log("Control loop completed");
    }
};

} // namespace good_design
