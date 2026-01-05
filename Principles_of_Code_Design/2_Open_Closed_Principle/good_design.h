#pragma once

#include <string>
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

namespace good_design {

/**
 * 好设计示例：使用抽象基类和多态遵循开闭原则
 * 
 * 优势：添加新协议只需创建新的Communicator类，不需要修改现有代码
 * 符合开闭原则：对扩展开放，对修改关闭
 */

// 抽象协议通信器接口
class IProtocolCommunicator {
public:
    virtual ~IProtocolCommunicator() = default;
    
    // 纯虚函数，子类必须实现
    virtual bool connect() = 0;
    virtual bool send(const std::string& data) = 0;
    virtual void disconnect() = 0;
    
    virtual std::string getProtocolName() const = 0;
    virtual bool isConnected() const = 0;
};

// MQTT协议实现
class MqttCommunicator : public IProtocolCommunicator {
private:
    bool connected_;
    
public:
    MqttCommunicator() : connected_(false) {
        std::cout << "[Good Design] MqttCommunicator created" << std::endl;
    }
    
    bool connect() override {
        std::cout << "[Good Design] Connecting via MQTT..." << std::endl;
        // 模拟MQTT连接
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "[Good Design] MQTT connected to broker: tcp://broker.example.com:1883" 
                  << std::endl;
        connected_ = true;
        return true;
    }
    
    bool send(const std::string& data) override {
        if (!connected_) {
            std::cout << "[Good Design] MQTT Error: Not connected" << std::endl;
            return false;
        }
        
        std::cout << "[Good Design] MQTT published to topic: device/data" << std::endl;
        std::cout << "[Good Design] MQTT payload: " << data << std::endl;
        return true;
    }
    
    void disconnect() override {
        std::cout << "[Good Design] Disconnecting MQTT..." << std::endl;
        std::cout << "[Good Design] MQTT disconnected" << std::endl;
        connected_ = false;
    }
    
    std::string getProtocolName() const override {
        return "MQTT";
    }
    
    bool isConnected() const override {
        return connected_;
    }
};

// HTTP协议实现
class HttpCommunicator : public IProtocolCommunicator {
private:
    bool connected_;
    
public:
    HttpCommunicator() : connected_(false) {
        std::cout << "[Good Design] HttpCommunicator created" << std::endl;
    }
    
    bool connect() override {
        std::cout << "[Good Design] Connecting via HTTP..." << std::endl;
        // 模拟HTTP连接
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::cout << "[Good Design] HTTP connected to server: http://api.example.com" 
                  << std::endl;
        connected_ = true;
        return true;
    }
    
    bool send(const std::string& data) override {
        if (!connected_) {
            std::cout << "[Good Design] HTTP Error: Not connected" << std::endl;
            return false;
        }
        
        std::cout << "[Good Design] HTTP POST to: http://api.example.com/data" << std::endl;
        std::cout << "[Good Design] HTTP payload: " << data << std::endl;
        return true;
    }
    
    void disconnect() override {
        std::cout << "[Good Design] Disconnecting HTTP..." << std::endl;
        std::cout << "[Good Design] HTTP disconnected" << std::endl;
        connected_ = false;
    }
    
    std::string getProtocolName() const override {
        return "HTTP";
    }
    
    bool isConnected() const override {
        return connected_;
    }
};

// CoAP协议实现
class CoapCommunicator : public IProtocolCommunicator {
private:
    bool connected_;
    
public:
    CoapCommunicator() : connected_(false) {
        std::cout << "[Good Design] CoapCommunicator created" << std::endl;
    }
    
    bool connect() override {
        std::cout << "[Good Design] Connecting via CoAP..." << std::endl;
        // 模拟CoAP连接
        std::this_thread::sleep_for(std::chrono::milliseconds(75));
        std::cout << "[Good Design] CoAP connected to server: coap://coap.example.com:5683" 
                  << std::endl;
        connected_ = true;
        return true;
    }
    
    bool send(const std::string& data) override {
        if (!connected_) {
            std::cout << "[Good Design] CoAP Error: Not connected" << std::endl;
            return false;
        }
        
        std::cout << "[Good Design] CoAP POST to: coap://coap.example.com/resource" << std::endl;
        std::cout << "[Good Design] CoAP payload: " << data << std::endl;
        return true;
    }
    
    void disconnect() override {
        std::cout << "[Good Design] Disconnecting CoAP..." << std::endl;
        std::cout << "[Good Design] CoAP disconnected" << std::endl;
        connected_ = false;
    }
    
    std::string getProtocolName() const override {
        return "CoAP";
    }
    
    bool isConnected() const override {
        return connected_;
    }
};

// WebSocket协议实现（演示添加新协议）
class WebSocketCommunicator : public IProtocolCommunicator {
private:
    bool connected_;
    
public:
    WebSocketCommunicator() : connected_(false) {
        std::cout << "[Good Design] WebSocketCommunicator created" << std::endl;
    }
    
    bool connect() override {
        std::cout << "[Good Design] Connecting via WebSocket..." << std::endl;
        // 模拟WebSocket连接
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
        std::cout << "[Good Design] WebSocket connected to server: ws://ws.example.com:8080" 
                  << std::endl;
        connected_ = true;
        return true;
    }
    
    bool send(const std::string& data) override {
        if (!connected_) {
            std::cout << "[Good Design] WebSocket Error: Not connected" << std::endl;
            return false;
        }
        
        std::cout << "[Good Design] WebSocket send message" << std::endl;
        std::cout << "[Good Design] WebSocket payload: " << data << std::endl;
        return true;
    }
    
    void disconnect() override {
        std::cout << "[Good Design] Disconnecting WebSocket..." << std::endl;
        std::cout << "[Good Design] WebSocket disconnected" << std::endl;
        connected_ = false;
    }
    
    std::string getProtocolName() const override {
        return "WebSocket";
    }
    
    bool isConnected() const override {
        return connected_;
    }
};

// 设备通信器（依赖注入，不需要修改）
class DeviceCommunicator {
private:
    std::unique_ptr<IProtocolCommunicator> protocol_;
    
public:
    DeviceCommunicator(std::unique_ptr<IProtocolCommunicator> protocol)
        : protocol_(std::move(protocol)) {
        std::cout << "[Good Design] DeviceCommunicator created with protocol: " 
                  << protocol_->getProtocolName() << std::endl;
    }
    
    // 连接到服务器
    bool connect() {
        if (!protocol_) {
            std::cout << "[Good Design] Error: No protocol set" << std::endl;
            return false;
        }
        
        std::cout << "[Good Design] Connecting via " 
                  << protocol_->getProtocolName() << "..." << std::endl;
        return protocol_->connect();
    }
    
    // 发送数据
    bool send(const std::string& data) {
        if (!protocol_) {
            std::cout << "[Good Design] Error: No protocol set" << std::endl;
            return false;
        }
        
        std::cout << "[Good Design] Sending data via " 
                  << protocol_->getProtocolName() << "..." << std::endl;
        return protocol_->send(data);
    }
    
    // 断开连接
    void disconnect() {
        if (!protocol_) {
            return;
        }
        
        std::cout << "[Good Design] Disconnecting from " 
                  << protocol_->getProtocolName() << "..." << std::endl;
        protocol_->disconnect();
    }
    
    bool isConnected() const {
        return protocol_ && protocol_->isConnected();
    }
    
    // 可以在运行时切换协议（可选功能）
    void setProtocol(std::unique_ptr<IProtocolCommunicator> protocol) {
        if (protocol_ && protocol_->isConnected()) {
            protocol_->disconnect();
        }
        protocol_ = std::move(protocol);
        std::cout << "[Good Design] Protocol switched to: " 
                  << protocol_->getProtocolName() << std::endl;
    }
};

} // namespace good_design
