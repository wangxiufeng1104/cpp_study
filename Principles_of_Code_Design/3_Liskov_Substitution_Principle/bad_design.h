#pragma once

#include <string>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>

namespace bad_design {

/**
 * 坏设计示例：违反里氏替换原则
 * 
 * 问题：子类违反了父类的行为契约，导致无法安全替换
 */

// 父类：通信器
class Communicator {
public:
    virtual ~Communicator() = default;
    
    // 契约说明：
    // 1. connect(): 返回true表示连接成功，false表示失败
    // 2. send(): 只有connected时才能发送数据，返回true表示成功，false表示失败
    // 3. disconnect(): 断开连接，之后isConnected()应返回false
    // 4. isConnected(): 返回连接状态
    virtual bool connect() = 0;
    virtual bool send(const std::string& data) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
};

// 子类1：MQTT通信器（正确实现）
class MqttCommunicator : public Communicator {
private:
    bool connected_;
    std::string broker_;
    int port_;
    
public:
    MqttCommunicator(const std::string& broker, int port)
        : connected_(false), broker_(broker), port_(port) {
        std::cout << "[Bad Design] MqttCommunicator created" << std::endl;
    }
    
    bool connect() override {
        std::cout << "[Bad Design] MQTT connecting to " << broker_ << ":" << port_ << "..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        connected_ = true;
        std::cout << "[Bad Design] MQTT connected" << std::endl;
        return true;
    }
    
    bool send(const std::string& data) override {
        if (!connected_) {
            std::cout << "[Bad Design] MQTT Error: Not connected" << std::endl;
            return false;
        }
        std::cout << "[Bad Design] MQTT sent: " << data << std::endl;
        return true;
    }
    
    void disconnect() override {
        std::cout << "[Bad Design] MQTT disconnecting..." << std::endl;
        connected_ = false;
        std::cout << "[Bad Design] MQTT disconnected" << std::endl;
    }
    
    bool isConnected() const override {
        return connected_;
    }
};

// 子类2：HTTP通信器（正确实现）
class HttpCommunicator : public Communicator {
private:
    bool connected_;
    std::string server_;
    int port_;
    
public:
    HttpCommunicator(const std::string& server, int port)
        : connected_(false), server_(server), port_(port) {
        std::cout << "[Bad Design] HttpCommunicator created" << std::endl;
    }
    
    bool connect() override {
        std::cout << "[Bad Design] HTTP connecting to " << server_ << ":" << port_ << "..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        connected_ = true;
        std::cout << "[Bad Design] HTTP connected" << std::endl;
        return true;
    }
    
    bool send(const std::string& data) override {
        if (!connected_) {
            std::cout << "[Bad Design] HTTP Error: Not connected" << std::endl;
            return false;
        }
        std::cout << "[Bad Design] HTTP POST: " << data << std::endl;
        return true;
    }
    
    void disconnect() override {
        std::cout << "[Bad Design] HTTP disconnecting..." << std::endl;
        connected_ = false;
        std::cout << "[Bad Design] HTTP disconnected" << std::endl;
    }
    
    bool isConnected() const override {
        return connected_;
    }
};

// 子类3：UART通信器（违反LSP的坏设计）
class UartCommunicator : public Communicator {
private:
    bool connected_;
    std::string port_;
    int baudRate_;
    int sendCount_;  // 发送计数
    
public:
    UartCommunicator(const std::string& port, int baudRate)
        : connected_(false), port_(port), baudRate_(baudRate), sendCount_(0) {
        std::cout << "[Bad Design] UartCommunicator created" << std::endl;
    }
    
    bool connect() override {
        std::cout << "[Bad Design] UART connecting to " << port_ << " at " 
                  << baudRate_ << " baud..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        connected_ = true;
        std::cout << "[Bad Design] UART connected" << std::endl;
        return true;
    }
    
    bool send(const std::string& data) override {
        sendCount_++;
        
        // 违反LSP问题1：抛出异常
        // 父类send()方法没有声明会抛出异常
        if (sendCount_ > 3) {
            throw std::runtime_error("UART busy after 3 sends");
        }
        
        if (!connected_) {
            std::cout << "[Bad Design] UART Error: Not connected" << std::endl;
            return false;
        }
        std::cout << "[Bad Design] UART sent: " << data << std::endl;
        return true;
    }
    
    void disconnect() override {
        std::cout << "[Bad Design] UART disconnecting..." << std::endl;
        // 违反LSP问题2：disconnect后connected仍为true
        // 违反了disconnect()后isConnected()应为false的契约
        connected_ = true;  // 这里故意设置为true，模拟坏设计
        std::cout << "[Bad Design] UART disconnected (but still reports connected!)" << std::endl;
    }
    
    bool isConnected() const override {
        return connected_;
    }
    
    // 违反LSP问题3：添加了父类没有的方法
    // 这不是LSP的直接问题，但增加了接口的不一致性
    void reset() {
        std::cout << "[Bad Design] UART resetting..." << std::endl;
        sendCount_ = 0;
    }
};

// 子类4：文件通信器（违反LSP的坏设计）
class FileCommunicator : public Communicator {
private:
    bool connected_;
    std::string filename_;
    
public:
    FileCommunicator(const std::string& filename)
        : connected_(false), filename_(filename) {
        std::cout << "[Bad Design] FileCommunicator created" << std::endl;
    }
    
    bool connect() override {
        std::cout << "[Bad Design] File opening " << filename_ << "..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        connected_ = true;
        std::cout << "[Bad Design] File opened" << std::endl;
        return true;
    }
    
    bool send(const std::string& data) override {
        if (!connected_) {
            std::cout << "[Bad Design] File Error: Not connected" << std::endl;
            return false;
        }
        std::cout << "[Bad Design] File write: " << data << std::endl;
        return true;
    }
    
    void disconnect() override {
        std::cout << "[Bad Design] File closing..." << std::endl;
        connected_ = false;
        std::cout << "[Bad Design] File closed" << std::endl;
    }
    
    // 违反LSP问题4：isConnected()返回值含义不同
    // 父类isConnected()应反映真实连接状态
    // 但这里总是返回true，表示文件已打开即可写入
    bool isConnected() const override {
        return true;  // 总是返回true，违反契约
    }
};

} // namespace bad_design
