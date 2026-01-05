#pragma once

#include <string>
#include <iostream>
#include <thread>
#include <chrono>

namespace good_design {

/**
 * 好设计示例：遵循里氏替换原则
 * 
 * 优势：所有子类都严格遵守父类的行为契约，可以安全替换
 */

// 父类：通信器（明确定义契约）
class Communicator {
public:
    virtual ~Communicator() = default;
    
    // 明确的契约说明：
    // 1. connect(): 返回true表示连接成功，false表示失败
    // 2. send(): 只有connected时才能发送数据，返回true表示成功，false表示失败
    // 3. disconnect(): 断开连接，之后isConnected()应返回false
    // 4. isConnected(): 返回真实连接状态
    virtual bool connect() = 0;
    virtual bool send(const std::string& data) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
};

// 子类1：MQTT通信器（严格遵循契约）
class MqttCommunicator : public Communicator {
private:
    bool connected_;
    std::string broker_;
    int port_;
    
public:
    MqttCommunicator(const std::string& broker, int port)
        : connected_(false), broker_(broker), port_(port) {
        std::cout << "[Good Design] MqttCommunicator created" << std::endl;
    }
    
    bool connect() override {
        std::cout << "[Good Design] MQTT connecting to " << broker_ << ":" << port_ << "..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        connected_ = true;
        std::cout << "[Good Design] MQTT connected" << std::endl;
        return true;
    }
    
    bool send(const std::string& data) override {
        if (!connected_) {
            std::cout << "[Good Design] MQTT Error: Not connected" << std::endl;
            return false;
        }
        std::cout << "[Good Design] MQTT sent: " << data << std::endl;
        return true;
    }
    
    void disconnect() override {
        std::cout << "[Good Design] MQTT disconnecting..." << std::endl;
        connected_ = false;  // 确保断开后连接状态为false
        std::cout << "[Good Design] MQTT disconnected" << std::endl;
    }
    
    bool isConnected() const override {
        return connected_;  // 返回真实连接状态
    }
};

// 子类2：HTTP通信器（严格遵循契约）
class HttpCommunicator : public Communicator {
private:
    bool connected_;
    std::string server_;
    int port_;
    
public:
    HttpCommunicator(const std::string& server, int port)
        : connected_(false), server_(server), port_(port) {
        std::cout << "[Good Design] HttpCommunicator created" << std::endl;
    }
    
    bool connect() override {
        std::cout << "[Good Design] HTTP connecting to " << server_ << ":" << port_ << "..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        connected_ = true;
        std::cout << "[Good Design] HTTP connected" << std::endl;
        return true;
    }
    
    bool send(const std::string& data) override {
        if (!connected_) {
            std::cout << "[Good Design] HTTP Error: Not connected" << std::endl;
            return false;
        }
        std::cout << "[Good Design] HTTP POST: " << data << std::endl;
        return true;
    }
    
    void disconnect() override {
        std::cout << "[Good Design] HTTP disconnecting..." << std::endl;
        connected_ = false;  // 确保断开后连接状态为false
        std::cout << "[Good Design] HTTP disconnected" << std::endl;
    }
    
    bool isConnected() const override {
        return connected_;  // 返回真实连接状态
    }
};

// 子类3：UART通信器（严格遵循契约）
class UartCommunicator : public Communicator {
private:
    bool connected_;
    std::string port_;
    int baudRate_;
    
public:
    UartCommunicator(const std::string& port, int baudRate)
        : connected_(false), port_(port), baudRate_(baudRate) {
        std::cout << "[Good Design] UartCommunicator created" << std::endl;
    }
    
    bool connect() override {
        std::cout << "[Good Design] UART connecting to " << port_ << " at " 
                  << baudRate_ << " baud..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        connected_ = true;
        std::cout << "[Good Design] UART connected" << std::endl;
        return true;
    }
    
    bool send(const std::string& data) override {
        if (!connected_) {
            std::cout << "[Good Design] UART Error: Not connected" << std::endl;
            return false;
        }
        // 不抛出异常，通过返回值报告错误
        std::cout << "[Good Design] UART sent: " << data << std::endl;
        return true;
    }
    
    void disconnect() override {
        std::cout << "[Good Design] UART disconnecting..." << std::endl;
        connected_ = false;  // 确保断开后连接状态为false
        std::cout << "[Good Design] UART disconnected" << std::endl;
    }
    
    bool isConnected() const override {
        return connected_;  // 返回真实连接状态
    }
};

// 子类4：文件通信器（严格遵循契约）
class FileCommunicator : public Communicator {
private:
    bool connected_;
    std::string filename_;
    
public:
    FileCommunicator(const std::string& filename)
        : connected_(false), filename_(filename) {
        std::cout << "[Good Design] FileCommunicator created" << std::endl;
    }
    
    bool connect() override {
        std::cout << "[Good Design] File opening " << filename_ << "..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        connected_ = true;
        std::cout << "[Good Design] File opened" << std::endl;
        return true;
    }
    
    bool send(const std::string& data) override {
        if (!connected_) {
            std::cout << "[Good Design] File Error: Not connected" << std::endl;
            return false;
        }
        std::cout << "[Good Design] File write: " << data << std::endl;
        return true;
    }
    
    void disconnect() override {
        std::cout << "[Good Design] File closing..." << std::endl;
        connected_ = false;  // 确保断开后连接状态为false
        std::cout << "[Good Design] File closed" << std::endl;
    }
    
    bool isConnected() const override {
        return connected_;  // 返回真实连接状态（而非总是true）
    }
};

// 子类5：WebSocket通信器（严格遵循契约）
class WebSocketCommunicator : public Communicator {
private:
    bool connected_;
    std::string server_;
    int port_;
    
public:
    WebSocketCommunicator(const std::string& server, int port)
        : connected_(false), server_(server), port_(port) {
        std::cout << "[Good Design] WebSocketCommunicator created" << std::endl;
    }
    
    bool connect() override {
        std::cout << "[Good Design] WebSocket connecting to " << server_ << ":" << port_ << "..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
        connected_ = true;
        std::cout << "[Good Design] WebSocket connected" << std::endl;
        return true;
    }
    
    bool send(const std::string& data) override {
        if (!connected_) {
            std::cout << "[Good Design] WebSocket Error: Not connected" << std::endl;
            return false;
        }
        std::cout << "[Good Design] WebSocket sent: " << data << std::endl;
        return true;
    }
    
    void disconnect() override {
        std::cout << "[Good Design] WebSocket disconnecting..." << std::endl;
        connected_ = false;  // 确保断开后连接状态为false
        std::cout << "[Good Design] WebSocket disconnected" << std::endl;
    }
    
    bool isConnected() const override {
        return connected_;  // 返回真实连接状态
    }
};

} // namespace good_design
