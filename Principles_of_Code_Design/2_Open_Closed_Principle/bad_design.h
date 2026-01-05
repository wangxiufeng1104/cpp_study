#pragma once

#include <string>
#include <iostream>
#include <thread>
#include <chrono>

namespace bad_design {

/**
 * 坏设计示例：DeviceCommunicator违反开闭原则
 * 
 * 问题：使用if-else或switch-case判断协议类型
 * 添加新协议需要修改现有代码，违反开闭原则
 */
class DeviceCommunicator {
public:
    enum class Protocol {
        MQTT,
        HTTP,
        COAP
    };
    
private:
    Protocol protocol_;
    bool connected_;
    
public:
    DeviceCommunicator(Protocol protocol) 
        : protocol_(protocol), connected_(false) {
        std::cout << "[Bad Design] DeviceCommunicator created with protocol: " 
                  << getProtocolName(protocol_) << std::endl;
    }
    
    // 连接到服务器
    bool connect() {
        std::cout << "[Bad Design] Connecting via " 
                  << getProtocolName(protocol_) << "..." << std::endl;
        
        // 违反开闭原则：添加新协议需要修改这里
        switch (protocol_) {
            case Protocol::MQTT:
                connected_ = connectMQTT();
                break;
            case Protocol::HTTP:
                connected_ = connectHTTP();
                break;
            case Protocol::COAP:
                connected_ = connectCoAP();
                break;
            // 如果要添加WebSocket，需要在这里添加新的case
        }
        
        return connected_;
    }
    
    // 发送数据
    bool send(const std::string& data) {
        if (!connected_) {
            std::cout << "[Bad Design] Error: Not connected" << std::endl;
            return false;
        }
        
        std::cout << "[Bad Design] Sending data via " 
                  << getProtocolName(protocol_) << "..." << std::endl;
        
        // 违反开闭原则：添加新协议需要修改这里
        switch (protocol_) {
            case Protocol::MQTT:
                return sendViaMQTT(data);
            case Protocol::HTTP:
                return sendViaHTTP(data);
            case Protocol::COAP:
                return sendViaCoAP(data);
            // 如果要添加WebSocket，需要在这里添加新的case
        }
        
        return false;
    }
    
    // 断开连接
    void disconnect() {
        std::cout << "[Bad Design] Disconnecting from " 
                  << getProtocolName(protocol_) << "..." << std::endl;
        
        // 违反开闭原则：添加新协议需要修改这里
        switch (protocol_) {
            case Protocol::MQTT:
                disconnectMQTT();
                break;
            case Protocol::HTTP:
                disconnectHTTP();
                break;
            case Protocol::COAP:
                disconnectCoAP();
                break;
            // 如果要添加WebSocket，需要在这里添加新的case
        }
        
        connected_ = false;
    }
    
    bool isConnected() const {
        return connected_;
    }
    
private:
    // MQTT相关方法
    bool connectMQTT() {
        // 模拟MQTT连接
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "[Bad Design] MQTT connected to broker: tcp://broker.example.com:1883" 
                  << std::endl;
        return true;
    }
    
    bool sendViaMQTT(const std::string& data) {
        std::cout << "[Bad Design] MQTT published to topic: device/data" << std::endl;
        std::cout << "[Bad Design] MQTT payload: " << data << std::endl;
        return true;
    }
    
    void disconnectMQTT() {
        std::cout << "[Bad Design] MQTT disconnected" << std::endl;
    }
    
    // HTTP相关方法
    bool connectHTTP() {
        // 模拟HTTP连接
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::cout << "[Bad Design] HTTP connected to server: http://api.example.com" 
                  << std::endl;
        return true;
    }
    
    bool sendViaHTTP(const std::string& data) {
        std::cout << "[Bad Design] HTTP POST to: http://api.example.com/data" << std::endl;
        std::cout << "[Bad Design] HTTP payload: " << data << std::endl;
        return true;
    }
    
    void disconnectHTTP() {
        std::cout << "[Bad Design] HTTP disconnected" << std::endl;
    }
    
    // CoAP相关方法
    bool connectCoAP() {
        // 模拟CoAP连接
        std::this_thread::sleep_for(std::chrono::milliseconds(75));
        std::cout << "[Bad Design] CoAP connected to server: coap://coap.example.com:5683" 
                  << std::endl;
        return true;
    }
    
    bool sendViaCoAP(const std::string& data) {
        std::cout << "[Bad Design] CoAP POST to: coap://coap.example.com/resource" << std::endl;
        std::cout << "[Bad Design] CoAP payload: " << data << std::endl;
        return true;
    }
    
    void disconnectCoAP() {
        std::cout << "[Bad Design] CoAP disconnected" << std::endl;
    }
    
    // 获取协议名称
    std::string getProtocolName(Protocol protocol) {
        switch (protocol) {
            case Protocol::MQTT: return "MQTT";
            case Protocol::HTTP: return "HTTP";
            case Protocol::COAP: return "CoAP";
        }
        return "Unknown";
    }
};

} // namespace bad_design
