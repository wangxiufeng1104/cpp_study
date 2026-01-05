#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include "bad_design.h"
#include "good_design.h"

void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << title << std::endl;
    std::cout << std::string(70, '=') << std::endl;
}

void demonstrateBadDesign() {
    printSeparator("坏设计演示 - 违反开闭原则");
    
    std::cout << "\n【场景1：使用MQTT协议】" << std::endl;
    {
        bad_design::DeviceCommunicator mqttDevice(bad_design::DeviceCommunicator::Protocol::MQTT);
        
        if (mqttDevice.connect()) {
            mqttDevice.send("{\"temp\":25.5,\"humidity\":60.0}");
            mqttDevice.disconnect();
        }
    }
    
    std::cout << "\n【场景2：使用HTTP协议】" << std::endl;
    {
        bad_design::DeviceCommunicator httpDevice(bad_design::DeviceCommunicator::Protocol::HTTP);
        
        if (httpDevice.connect()) {
            httpDevice.send("{\"status\":\"online\",\"version\":\"1.0\"}");
            httpDevice.disconnect();
        }
    }
    
    std::cout << "\n【场景3：使用CoAP协议】" << std::endl;
    {
        bad_design::DeviceCommunicator coapDevice(bad_design::DeviceCommunicator::Protocol::COAP);
        
        if (coapDevice.connect()) {
            coapDevice.send("{\"action\":\"heartbeat\"}");
            coapDevice.disconnect();
        }
    }
    
    std::cout << "\n【问题分析】" << std::endl;
    std::cout << "如果要添加新的WebSocket协议，需要：" << std::endl;
    std::cout << "1. 修改DeviceCommunicator::Protocol枚举，添加WebSocket" << std::endl;
    std::cout << "2. 修改connect()方法，添加WebSocket的case" << std::endl;
    std::cout << "3. 修改send()方法，添加WebSocket的case" << std::endl;
    std::cout << "4. 修改disconnect()方法，添加WebSocket的case" << std::endl;
    std::cout << "5. 实现connectWebSocket()、sendViaWebSocket()、disconnectWebSocket()" << std::endl;
    std::cout << "\n违反开闭原则：" << std::endl;
    std::cout << "✗ 对扩展不开放：添加新协议需要修改现有代码" << std::endl;
    std::cout << "✗ 对修改开放：每次添加新功能都要修改DeviceCommunicator类" << std::endl;
    std::cout << "✗ 代码耦合：所有协议逻辑都在一个类中" << std::endl;
    std::cout << "✗ 难以维护：类变得庞大且复杂" << std::endl;
    std::cout << "✗ 测试困难：修改一个协议可能影响其他协议" << std::endl;
}

void demonstrateGoodDesign() {
    printSeparator("好设计演示 - 遵循开闭原则");
    
    std::cout << "\n【场景1：使用MQTT协议】" << std::endl;
    {
        auto mqttComm = std::make_unique<good_design::MqttCommunicator>();
        good_design::DeviceCommunicator mqttDevice(std::move(mqttComm));
        
        if (mqttDevice.connect()) {
            mqttDevice.send("{\"temp\":25.5,\"humidity\":60.0}");
            mqttDevice.disconnect();
        }
    }
    
    std::cout << "\n【场景2：使用HTTP协议】" << std::endl;
    {
        auto httpComm = std::make_unique<good_design::HttpCommunicator>();
        good_design::DeviceCommunicator httpDevice(std::move(httpComm));
        
        if (httpDevice.connect()) {
            httpDevice.send("{\"status\":\"online\",\"version\":\"1.0\"}");
            httpDevice.disconnect();
        }
    }
    
    std::cout << "\n【场景3：使用CoAP协议】" << std::endl;
    {
        auto coapComm = std::make_unique<good_design::CoapCommunicator>();
        good_design::DeviceCommunicator coapDevice(std::move(coapComm));
        
        if (coapDevice.connect()) {
            coapDevice.send("{\"action\":\"heartbeat\"}");
            coapDevice.disconnect();
        }
    }
    
    std::cout << "\n【场景4：添加WebSocket协议（新增）】" << std::endl;
    {
        auto wsComm = std::make_unique<good_design::WebSocketCommunicator>();
        good_design::DeviceCommunicator wsDevice(std::move(wsComm));
        
        if (wsDevice.connect()) {
            wsDevice.send("{\"event\":\"device_ready\"}");
            wsDevice.disconnect();
        }
    }
    
    std::cout << "\n【优势分析】" << std::endl;
    std::cout << "添加WebSocket协议只需要：" << std::endl;
    std::cout << "1. 创建WebSocketCommunicator类（继承IProtocolCommunicator）" << std::endl;
    std::cout << "2. 实现connect()、send()、disconnect()方法" << std::endl;
    std::cout << "3. DeviceCommunicator类完全不需要修改！" << std::endl;
    std::cout << "\n符合开闭原则：" << std::endl;
    std::cout << "✓ 对扩展开放：添加新协议只需创建新类" << std::endl;
    std::cout << "✓ 对修改关闭：DeviceCommunicator类无需修改" << std::endl;
    std::cout << "✓ 代码解耦：每个协议独立实现" << std::endl;
    std::cout << "✓ 易于维护：每个协议类职责单一" << std::endl;
    std::cout << "✓ 易于测试：可以独立测试每个协议" << std::endl;
    std::cout << "✓ 运行时切换：可以动态切换通信协议" << std::endl;
}

void demonstrateRuntimeSwitch() {
    printSeparator("运行时协议切换演示");
    
    std::cout << "\n创建设备并使用MQTT协议..." << std::endl;
    auto mqttComm = std::make_unique<good_design::MqttCommunicator>();
    good_design::DeviceCommunicator device(std::move(mqttComm));
    
    device.connect();
    device.send("{\"protocol\":\"mqtt\",\"data\":\"first message\"}");
    
    std::cout << "\n运行时切换到HTTP协议..." << std::endl;
    auto httpComm = std::make_unique<good_design::HttpCommunicator>();
    device.setProtocol(std::move(httpComm));
    
    device.connect();
    device.send("{\"protocol\":\"http\",\"data\":\"second message\"}");
    
    std::cout << "\n运行时切换到WebSocket协议..." << std::endl;
    auto wsComm = std::make_unique<good_design::WebSocketCommunicator>();
    device.setProtocol(std::move(wsComm));
    
    device.connect();
    device.send("{\"protocol\":\"websocket\",\"data\":\"third message\"}");
    
    device.disconnect();
    
    std::cout << "\n【运行时切换的优势】" << std::endl;
    std::cout << "• 可以根据网络条件动态选择协议" << std::endl;
    std::cout << "• MQTT不稳定时切换到HTTP" << std::endl;
    std::cout << "• 需要实时通信时切换到WebSocket" << std::endl;
    std::cout << "• 资源受限时切换到CoAP" << std::endl;
}

void demonstrateComparison() {
    printSeparator("设计对比分析");
    
    std::cout << "\n【代码复杂度对比】" << std::endl;
    std::cout << "坏设计 (switch-case):" << std::endl;
    std::cout << "  - 1个大类，约200行代码" << std::endl;
    std::cout << "  - 包含所有协议的实现" << std::endl;
    std::cout << "  - 添加新协议需要修改多个方法" << std::endl;
    
    std::cout << "\n好设计 (多态):" << std::endl;
    std::cout << "  - 1个接口 + 4个实现类" << std::endl;
    std::cout << "  - 每个协议独立，约50行代码/类" << std::endl;
    std::cout << "  - 添加新协议无需修改现有代码" << std::endl;
    
    std::cout << "\n【可维护性对比】" << std::endl;
    std::cout << "坏设计:" << std::endl;
    std::cout << "  - 修改一个协议可能影响其他协议" << std::endl;
    std::cout << "  - 需要重新测试整个类" << std::endl;
    std::cout << "  - 代码审查困难，逻辑复杂" << std::endl;
    
    std::cout << "\n好设计:" << std::endl;
    std::cout << "  - 修改一个协议不影响其他协议" << std::endl;
    std::cout << "  - 只需测试修改的协议类" << std::endl;
    std::cout << "  - 代码结构清晰，易于理解" << std::endl;
    
    std::cout << "\n【扩展性对比】" << std::endl;
    std::cout << "坏设计:" << std::endl;
    std::cout << "  - 添加新协议需要修改核心代码" << std::endl;
    std::cout << "  - 增加回归测试的成本" << std::endl;
    std::cout << "  - 可能引入新的bug" << std::endl;
    
    std::cout << "\n好设计:" << std::endl;
    std::cout << "  - 添加新协议只需创建新类" << std::endl;
    std::cout << "  - 只需测试新增的类" << std::endl;
    std::cout << "  - 不会引入新的bug到现有代码" << std::endl;
}

int main() {
    std::cout << "=== 开闭原则演示程序 ===" << std::endl;
    std::cout << "Open-Closed Principle Demonstration" << std::endl;
    std::cout << "场景：嵌入式设备通信协议扩展" << std::endl;
    
    try {
        // 演示坏设计
        demonstrateBadDesign();
        
        std::cout << "\n等待2秒..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // 演示好设计
        demonstrateGoodDesign();
        
        std::cout << "\n等待2秒..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // 演示运行时切换
        demonstrateRuntimeSwitch();
        
        std::cout << "\n等待2秒..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // 对比分析
        demonstrateComparison();
        
        printSeparator("演示完成");
        std::cout << "\n【开闭原则核心思想】" << std::endl;
        std::cout << "软件实体应该对扩展开放，对修改关闭。" << std::endl;
        std::cout << "\n【关键要点】" << std::endl;
        std::cout << "1. 通过抽象（接口）隔离变化" << std::endl;
        std::cout << "2. 使用多态实现运行时行为" << std::endl;
        std::cout << "3. 依赖倒置：依赖抽象而非具体实现" << std::endl;
        std::cout << "4. 添加新功能通过扩展而非修改" << std::endl;
        std::cout << "\n【实际应用】" << std::endl;
        std::cout << "• 插件架构" << std::endl;
        std::cout << "• 策略模式" << std::endl;
        std::cout << "• 工厂模式" << std::endl;
        std::cout << "• 模板方法模式" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "程序执行出错: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
