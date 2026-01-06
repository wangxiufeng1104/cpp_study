#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <memory>

namespace good_design {

// ============================================================================
// 职责1: 传感器读取器 - 只负责传感器数据采集
// ============================================================================

/**
 * ISensorReader接口 - 传感器读取器接口
 * 
 * 职责：只负责传感器数据读取
 * 
 * 优点：
 * - 可以独立测试
 * - 可以轻松替换不同的传感器实现
 * - 不依赖其他功能（MQTT、日志）
 */
class ISensorReader {
public:
    virtual ~ISensorReader() = default;
    
    struct SensorData {
        float temperature;
        float humidity;
        std::string timestamp;
    };
    
    virtual SensorData read() = 0;
    virtual bool initialize() = 0;
    virtual void cleanup() = 0;
};

/**
 * DHT11SensorReader - DHT11传感器实现
 * 
 * 职责：只负责DHT11传感器的数据读取
 */
class DHT11SensorReader : public ISensorReader {
private:
    std::string deviceId_;
    
public:
    explicit DHT11SensorReader(const std::string& deviceId = "sensor_001")
        : deviceId_(deviceId) {}
    
    bool initialize() override {
        std::cout << "[SensorReader] DHT11 sensor initialized: " << deviceId_ << std::endl;
        return true;
    }
    
    SensorData read() override {
        SensorData data;
        
        // 模拟读取DHT11传感器
        data.temperature = 20.0f + (rand() % 100) / 10.0f;
        data.humidity = 40.0f + (rand() % 400) / 10.0f;
        data.timestamp = getCurrentTimestamp();
        
        std::cout << "[SensorReader] DHT11 read: " 
                  << data.temperature << "°C, " 
                  << data.humidity << "%" << std::endl;
        
        return data;
    }
    
    void cleanup() override {
        std::cout << "[SensorReader] DHT11 sensor cleanup: " << deviceId_ << std::endl;
    }
    
private:
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::string ts = std::ctime(&time);
        ts.pop_back(); // 移除换行符
        return ts;
    }
};

/**
 * SHT30SensorReader - SHT30传感器实现（I2C接口）
 * 
 * 职责：只负责SHT30传感器的数据读取
 * 
 * 示例：展示如何轻松扩展新的传感器类型
 */
class SHT30SensorReader : public ISensorReader {
private:
    std::string deviceId_;
    
public:
    explicit SHT30SensorReader(const std::string& deviceId = "sensor_001")
        : deviceId_(deviceId) {}
    
    bool initialize() override {
        std::cout << "[SensorReader] SHT30 sensor initialized (I2C): " << deviceId_ << std::endl;
        return true;
    }
    
    SensorData read() override {
        SensorData data;
        
        // 模拟读取SHT30传感器（更精确）
        data.temperature = 20.0f + (rand() % 100) / 20.0f; // 更精确
        data.humidity = 40.0f + (rand() % 400) / 20.0f;   // 更精确
        data.timestamp = getCurrentTimestamp();
        
        std::cout << "[SensorReader] SHT30 read: " 
                  << data.temperature << "°C, " 
                  << data.humidity << "%" << std::endl;
        
        return data;
    }
    
    void cleanup() override {
        std::cout << "[SensorReader] SHT30 sensor cleanup: " << deviceId_ << std::endl;
    }
    
private:
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::string ts = std::ctime(&time);
        ts.pop_back();
        return ts;
    }
};

// ============================================================================
// 职责2: MQTT发布器 - 只负责数据上报
// ============================================================================

/**
 * IMqttPublisher接口 - MQTT发布器接口
 * 
 * 职责：只负责MQTT通信和数据上报
 * 
 * 优点：
 * - 可以独立测试
 * - 可以轻松替换不同的MQTT实现
 * - 不依赖其他功能（传感器、日志）
 */
class IMqttPublisher {
public:
    virtual ~IMqttPublisher() = default;
    
    virtual bool connect() = 0;
    virtual bool publish(const std::string& topic, const std::string& payload) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
};

/**
 * StandardMqttPublisher - 标准MQTT发布器
 * 
 * 职责：只负责标准MQTT协议的数据上报
 */
class StandardMqttPublisher : public IMqttPublisher {
private:
    bool connected_;
    std::string brokerUrl_;
    std::string clientId_;
    
public:
    StandardMqttPublisher(const std::string& brokerUrl = "mqtt://localhost:1883",
                        const std::string& clientId = "client_001")
        : connected_(false), brokerUrl_(brokerUrl), clientId_(clientId) {}
    
    bool connect() override {
        // 模拟MQTT连接
        connected_ = true;
        std::cout << "[MqttPublisher] Connected to broker: " << brokerUrl_ 
                  << " (client: " << clientId_ << ")" << std::endl;
        return connected_;
    }
    
    bool publish(const std::string& topic, const std::string& payload) override {
        if (!connected_) {
            std::cout << "[MqttPublisher] Error: Not connected, cannot publish" << std::endl;
            return false;
        }
        
        std::cout << "[MqttPublisher] Published to [" << topic << "]: " 
                  << payload << std::endl;
        return true;
    }
    
    void disconnect() override {
        connected_ = false;
        std::cout << "[MqttPublisher] Disconnected from broker" << std::endl;
    }
    
    bool isConnected() const override {
        return connected_;
    }
};

/**
 * AliyunMqttPublisher - 阿里云IoT平台MQTT发布器
 * 
 * 职责：只负责阿里云IoT平台的MQTT协议数据上报
 * 
 * 示例：展示如何轻松扩展不同的MQTT平台
 */
class AliyunMqttPublisher : public IMqttPublisher {
private:
    bool connected_;
    std::string productKey_;
    std::string deviceName_;
    
public:
    AliyunMqttPublisher(const std::string& productKey = "product_001",
                       const std::string& deviceName = "device_001")
        : connected_(false), productKey_(productKey), deviceName_(deviceName) {}
    
    bool connect() override {
        // 模拟阿里云IoT连接（包含认证）
        connected_ = true;
        std::cout << "[MqttPublisher] Connected to Aliyun IoT: " 
                  << productKey_ << "/" << deviceName_ << std::endl;
        return connected_;
    }
    
    bool publish(const std::string& topic, const std::string& payload) override {
        if (!connected_) {
            std::cout << "[MqttPublisher] Error: Not connected to Aliyun IoT" << std::endl;
            return false;
        }
        
        // 阿里云Topic格式
        std::string aliyunTopic = "/sys/" + productKey_ + "/" + 
                                deviceName_ + "/thing/event/property/post";
        
        std::cout << "[MqttPublisher] Published to [" << aliyunTopic << "]: " 
                  << payload << std::endl;
        return true;
    }
    
    void disconnect() override {
        connected_ = false;
        std::cout << "[MqttPublisher] Disconnected from Aliyun IoT" << std::endl;
    }
    
    bool isConnected() const override {
        return connected_;
    }
};

/**
 * HttpPublisher - HTTP协议发布器
 * 
 * 职责：只负责HTTP协议的数据上报
 * 
 * 示例：展示如何支持多种通信协议
 */
class HttpPublisher : public IMqttPublisher {
private:
    bool connected_;
    std::string apiUrl_;
    
public:
    explicit HttpPublisher(const std::string& apiUrl = "http://localhost:8080/api/sensor")
        : connected_(false), apiUrl_(apiUrl) {}
    
    bool connect() override {
        // HTTP是无状态的，不需要连接
        connected_ = true;
        std::cout << "[HttpPublisher] Ready to send HTTP requests to: " << apiUrl_ << std::endl;
        return connected_;
    }
    
    bool publish(const std::string& topic, const std::string& payload) override {
        if (!connected_) {
            std::cout << "[HttpPublisher] Error: Not ready" << std::endl;
            return false;
        }
        
        // 模拟HTTP POST请求
        std::cout << "[HttpPublisher] POST to " << apiUrl_ 
                  << " (topic: " << topic << "): " << payload << std::endl;
        return true;
    }
    
    void disconnect() override {
        connected_ = false;
        std::cout << "[HttpPublisher] HTTP publisher stopped" << std::endl;
    }
    
    bool isConnected() const override {
        return connected_;
    }
};

// ============================================================================
// 职责3: 设备日志器 - 只负责日志记录
// ============================================================================

/**
 * IDeviceLogger接口 - 设备日志器接口
 * 
 * 职责：只负责日志记录
 * 
 * 优点：
 * - 可以独立测试
 * - 可以轻松替换不同的日志实现
 * - 不依赖其他功能（传感器、MQTT）
 */
class IDeviceLogger {
public:
    virtual ~IDeviceLogger() = default;
    
    virtual void info(const std::string& message) = 0;
    virtual void error(const std::string& message) = 0;
    virtual void warning(const std::string& message) = 0;
};

/**
 * FileDeviceLogger - 文件日志器
 * 
 * 职责：只负责文件日志记录
 */
class FileDeviceLogger : public IDeviceLogger {
private:
    std::ofstream logFile_;
    std::string filename_;
    
public:
    explicit FileDeviceLogger(const std::string& filename = "device.log") 
        : filename_(filename) {
        logFile_.open(filename, std::ios::app);
        info("FileLogger initialized: " + filename);
    }
    
    ~FileDeviceLogger() {
        if (logFile_.is_open()) {
            info("FileLogger shutdown: " + filename_);
            logFile_.close();
        }
    }
    
    void info(const std::string& message) override {
        logMessage("[INFO]", message);
    }
    
    void error(const std::string& message) override {
        logMessage("[ERROR]", message);
    }
    
    void warning(const std::string& message) override {
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
};

/**
 * ConsoleDeviceLogger - 控制台日志器
 * 
 * 职责：只负责控制台日志输出
 */
class ConsoleDeviceLogger : public IDeviceLogger {
public:
    ConsoleDeviceLogger() {
        info("ConsoleLogger initialized");
    }
    
    void info(const std::string& message) override {
        logMessage("[INFO]", message);
    }
    
    void error(const std::string& message) override {
        logMessage("[ERROR]", message);
    }
    
    void warning(const std::string& message) override {
        logMessage("[WARNING]", message);
    }
    
private:
    void logMessage(const std::string& level, const std::string& message) {
        std::cout << level << " " << message << std::endl;
    }
};

// ============================================================================
// 职责4: 业务逻辑控制器 - 只负责协调各个组件
// ============================================================================

/**
 * SensorController - 传感器控制器
 * 
 * 职责：只负责业务逻辑协调，协调各个组件完成设备功能
 * 
 * 优点：
 * - 职责单一：只负责协调
 * - 通过依赖注入实现松耦合
 * - 各个组件可以独立替换
 * - 业务逻辑清晰，易于理解
 */
class SensorController {
private:
    std::unique_ptr<ISensorReader> sensorReader_;
    std::unique_ptr<IMqttPublisher> mqttPublisher_;
    std::unique_ptr<IDeviceLogger> logger_;
    std::string deviceId_;
    
public:
    SensorController(std::unique_ptr<ISensorReader> sensorReader,
                   std::unique_ptr<IMqttPublisher> mqttPublisher,
                   std::unique_ptr<IDeviceLogger> logger,
                   const std::string& deviceId = "sensor_001")
        : sensorReader_(std::move(sensorReader))
        , mqttPublisher_(std::move(mqttPublisher))
        , logger_(std::move(logger))
        , deviceId_(deviceId) {}
    
    void initialize() {
        logger_->info("Initializing SensorController: " + deviceId_);
        
        if (!sensorReader_->initialize()) {
            logger_->error("Failed to initialize sensor reader");
            return;
        }
        
        if (!mqttPublisher_->connect()) {
            logger_->error("Failed to connect MQTT publisher");
            return;
        }
        
        logger_->info("SensorController initialized successfully");
    }
    
    void runControlLoop(int iterations = 5) {
        logger_->info("Starting control loop with " + std::to_string(iterations) + " iterations");
        
        for (int i = 0; i < iterations; ++i) {
            logger_->info("=== Iteration " + std::to_string(i + 1) + " ===");
            
            // 步骤1: 使用传感器读取器读取数据
            auto data = sensorReader_->read();
            logger_->info("Read sensor data - Temp: " + std::to_string(data.temperature) + 
                         "°C, Humidity: " + std::to_string(data.humidity) + "%");
            
            // 步骤2: 使用MQTT发布器发布数据
            std::string topic = "sensors/" + deviceId_ + "/data";
            std::string payload = "{\"device\":\"" + deviceId_ + 
                              "\",\"temp\":" + std::to_string(data.temperature) + 
                              ",\"humidity\":" + std::to_string(data.humidity) + 
                              ",\"timestamp\":\"" + data.timestamp + "\"}";
            
            if (!mqttPublisher_->publish(topic, payload)) {
                logger_->error("Failed to publish data at iteration " + std::to_string(i + 1));
            }
            
            // 模拟设备休眠
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        logger_->info("Control loop completed");
    }
    
    void shutdown() {
        logger_->info("Shutting down SensorController: " + deviceId_);
        
        mqttPublisher_->disconnect();
        sensorReader_->cleanup();
        
        logger_->info("SensorController shutdown completed");
    }
    
    // 支持运行时替换组件
    void setSensorReader(std::unique_ptr<ISensorReader> reader) {
        logger_->info("Replacing sensor reader");
        sensorReader_->cleanup();
        sensorReader_ = std::move(reader);
        sensorReader_->initialize();
    }
    
    void setMqttPublisher(std::unique_ptr<IMqttPublisher> publisher) {
        logger_->info("Replacing MQTT publisher");
        mqttPublisher_->disconnect();
        mqttPublisher_ = std::move(publisher);
        mqttPublisher_->connect();
    }
    
    void setLogger(std::unique_ptr<IDeviceLogger> logger) {
        logger_->info("Replacing logger");
        logger_ = std::move(logger);
    }
};

} // namespace good_design
