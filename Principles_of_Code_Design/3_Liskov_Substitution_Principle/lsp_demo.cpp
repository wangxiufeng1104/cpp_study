#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <vector>
#include "bad_design.h"
#include "good_design.h"

void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << title << std::endl;
    std::cout << std::string(70, '=') << std::endl;
}

// 坏设计的通用使用函数（期望所有Communicator行为一致）
void useCommunicatorBad(bad_design::Communicator& comm, const std::string& name) {
    std::cout << "\n使用 " << name << " 通信器..." << std::endl;
    
    // 步骤1：连接
    if (!comm.connect()) {
        std::cout << "  连接失败！" << std::endl;
        return;
    }
    
    // 步骤2：发送数据
    comm.send("{\"temp\":25.5,\"humidity\":60.0}");
    comm.send("{\"status\":\"online\"}");
    comm.send("{\"action\":\"heartbeat\"}");
    
    // 步骤3：断开连接
    comm.disconnect();
    
    // 步骤4：验证断开状态
    // 如果子类违反LSP，这里会出错
    if (comm.isConnected()) {
        std::cout << "  ⚠️ 警告：断开后仍然报告已连接！违反LSP！" << std::endl;
    } else {
        std::cout << "  ✓ 已确认断开连接" << std::endl;
    }
}

void demonstrateBadDesign() {
    printSeparator("坏设计演示 - 违反里氏替换原则");
    
    std::cout << "\n【场景1：MQTT通信器（正常实现）】" << std::endl;
    {
        bad_design::MqttCommunicator mqtt("broker.example.com", 1883);
        useCommunicatorBad(mqtt, "MQTT");
    }
    
    std::cout << "\n【场景2：HTTP通信器（正常实现）】" << std::endl;
    {
        bad_design::HttpCommunicator http("api.example.com", 80);
        useCommunicatorBad(http, "HTTP");
    }
    
    std::cout << "\n【场景3：UART通信器（违反LSP问题1-2）】" << std::endl;
    {
        bad_design::UartCommunicator uart("/dev/ttyUSB0", 115200);
        
        std::cout << "\n使用 UART 通信器..." << std::endl;
        uart.connect();
        uart.send("{\"data\":1}");
        uart.send("{\"data\":2}");
        uart.send("{\"data\":3}");
        
        // 问题1：第4次发送会抛出异常
        std::cout << "\n尝试第4次发送..." << std::endl;
        try {
            uart.send("{\"data\":4}");
        } catch (const std::runtime_error& e) {
            std::cout << "  ⚠️ 异常：" << e.what() << std::endl;
            std::cout << "  ⚠️ 违反LSP：父类send()不应抛出异常！" << std::endl;
        }
        
        // 问题2：断开后仍然报告已连接
        std::cout << "\n断开UART连接..." << std::endl;
        uart.disconnect();
        if (uart.isConnected()) {
            std::cout << "  ⚠️ 违反LSP：disconnect()后isConnected()应为false！" << std::endl;
        }
    }
    
    std::cout << "\n【场景4：文件通信器（违反LSP问题4）】" << std::endl;
    {
        bad_design::FileCommunicator file("log.txt");
        
        std::cout << "\n使用文件通信器..." << std::endl;
        file.connect();
        file.send("{\"log\":\"entry1\"}");
        
        std::cout << "\n断开文件连接..." << std::endl;
        file.disconnect();
        
        // 问题：isConnected()总是返回true
        std::cout << "\n检查连接状态..." << std::endl;
        if (file.isConnected()) {
            std::cout << "  ⚠️ 违反LSP：isConnected()总是返回true！" << std::endl;
            std::cout << "  ⚠️ 违反LSP：isConnected()应反映真实连接状态！" << std::endl;
        }
    }
    
    std::cout << "\n【问题总结】" << std::endl;
    std::cout << "违反里氏替换原则的问题：" << std::endl;
    std::cout << "1. UART通信器：" << std::endl;
    std::cout << "   - send()抛出异常，父类未声明" << std::endl;
    std::cout << "   - disconnect()后isConnected()仍为true" << std::endl;
    std::cout << "   - 添加了父类没有的reset()方法" << std::endl;
    std::cout << "2. 文件通信器：" << std::endl;
    std::cout << "   - isConnected()总是返回true，不反映真实状态" << std::endl;
    std::cout << "\n后果：" << std::endl;
    std::cout << "✗ 子类无法安全替换父类" << std::endl;
    std::cout << "✗ 使用方需要特殊处理每个子类" << std::endl;
    std::cout << "✗ 代码复杂度增加" << std::endl;
    std::cout << "✗ 容易引入bug" << std::endl;
}

// 好设计的通用使用函数（所有Communicator行为一致）
void useCommunicatorGood(good_design::Communicator& comm, const std::string& name) {
    std::cout << "\n使用 " << name << " 通信器..." << std::endl;
    
    // 步骤1：连接
    if (!comm.connect()) {
        std::cout << "  连接失败！" << std::endl;
        return;
    }
    
    // 步骤2：发送数据
    comm.send("{\"temp\":25.5,\"humidity\":60.0}");
    comm.send("{\"status\":\"online\"}");
    comm.send("{\"action\":\"heartbeat\"}");
    
    // 步骤3：断开连接
    comm.disconnect();
    
    // 步骤4：验证断开状态
    // 所有子类都遵守契约，这里总是正确的
    if (comm.isConnected()) {
        std::cout << "  ⚠️ 警告：断开后仍然报告已连接！" << std::endl;
    } else {
        std::cout << "  ✓ 已确认断开连接" << std::endl;
    }
}

void demonstrateGoodDesign() {
    printSeparator("好设计演示 - 遵循里氏替换原则");
    
    std::cout << "\n【场景1：MQTT通信器】" << std::endl;
    {
        good_design::MqttCommunicator mqtt("broker.example.com", 1883);
        useCommunicatorGood(mqtt, "MQTT");
    }
    
    std::cout << "\n【场景2：HTTP通信器】" << std::endl;
    {
        good_design::HttpCommunicator http("api.example.com", 80);
        useCommunicatorGood(http, "HTTP");
    }
    
    std::cout << "\n【场景3：UART通信器】" << std::endl;
    {
        good_design::UartCommunicator uart("/dev/ttyUSB0", 115200);
        useCommunicatorGood(uart, "UART");
    }
    
    std::cout << "\n【场景4：文件通信器】" << std::endl;
    {
        good_design::FileCommunicator file("log.txt");
        useCommunicatorGood(file, "File");
    }
    
    std::cout << "\n【场景5：WebSocket通信器（新增）】" << std::endl;
    {
        good_design::WebSocketCommunicator ws("ws.example.com", 8080);
        useCommunicatorGood(ws, "WebSocket");
    }
    
    std::cout << "\n【优势总结】" << std::endl;
    std::cout << "遵循里氏替换原则的优势：" << std::endl;
    std::cout << "1. 所有子类严格遵守父类契约" << std::endl;
    std::cout << "2. 可以安全地用子类替换父类" << std::endl;
    std::cout << "3. 使用方代码一致，无需特殊处理" << std::endl;
    std::cout << "4. 降低了代码复杂度和bug风险" << std::endl;
    std::cout << "5. 提高了代码的可维护性和可测试性" << std::endl;
}

void demonstrateSubstitution() {
    printSeparator("替换原则演示");
    
    std::cout << "\n【演示：多态替换】" << std::endl;
    std::cout << "\n使用基类指针，可以指向任何子类..." << std::endl;
    
    std::vector<std::unique_ptr<good_design::Communicator>> communicators;
    communicators.push_back(std::make_unique<good_design::MqttCommunicator>("broker.example.com", 1883));
    communicators.push_back(std::make_unique<good_design::HttpCommunicator>("api.example.com", 80));
    communicators.push_back(std::make_unique<good_design::UartCommunicator>("/dev/ttyUSB0", 115200));
    communicators.push_back(std::make_unique<good_design::WebSocketCommunicator>("ws.example.com", 8080));
    
    std::cout << "\n遍历所有通信器（多态调用）..." << std::endl;
    for (auto& comm : communicators) {
        std::cout << "\n--- 使用通信器 ---" << std::endl;
        if (comm->connect()) {
            comm->send("{\"data\":\"test\"}");
            comm->disconnect();
            if (!comm->isConnected()) {
                std::cout << "  ✓ 正确断开" << std::endl;
            }
        }
    }
    
    std::cout << "\n【替换原则的价值】" << std::endl;
    std::cout << "✓ 所有子类都可以安全替换父类" << std::endl;
    std::cout << "✓ 使用方代码不需要知道具体类型" << std::endl;
    std::cout << "✓ 符合开闭原则：易于扩展" << std::endl;
    std::cout << "✓ 符合依赖倒置原则：依赖抽象" << std::endl;
}

void demonstrateComparison() {
    printSeparator("设计对比分析");
    
    std::cout << "\n【违反LSP的后果】" << std::endl;
    std::cout << "1. 违反类型契约" << std::endl;
    std::cout << "   - 改变方法的前置条件或后置条件" << std::endl;
    std::cout << "   - 抛出父类未声明的异常" << std::endl;
    std::cout << "   - 返回值含义与父类定义不同" << std::endl;
    
    std::cout << "\n2. 破坏多态行为" << std::endl;
    std::cout << "   - 子类无法透明替换父类" << std::endl;
    std::cout << "   - 需要if-else判断具体类型" << std::endl;
    std::cout << "   - 违反了面向对象多态的核心价值" << std::endl;
    
    std::cout << "\n3. 增加代码复杂度" << std::endl;
    std::cout << "   - 使用方需要特殊处理每个子类" << std::endl;
    std::cout << "   - 容易引入错误" << std::endl;
    std::cout << "   - 难以维护和测试" << std::endl;
    
    std::cout << "\n【遵循LSP的优势】" << std::endl;
    std::cout << "1. 类型安全" << std::endl;
    std::cout << "   - 所有子类都可以安全替换父类" << std::endl;
    std::cout << "   - 行为一致，可预测" << std::endl;
    
    std::cout << "\n2. 代码简洁" << std::endl;
    std::cout << "   - 使用方代码统一" << std::endl;
    std::cout << "   - 无需类型判断" << std::endl;
    std::cout << "   - 充分利用多态" << std::endl;
    
    std::cout << "\n3. 易于扩展" << std::endl;
    std::cout << "   - 添加新子类无需修改使用方" << std::endl;
    std::cout << "   - 符合开闭原则" << std::endl;
    
    std::cout << "\n【LSP与继承】" << std::endl;
    std::cout << "继承不仅仅是为了代码复用，更重要的是建立" << std::endl;
    std::cout << "『is-a』关系。子类必须完全符合父类的行为契约。" << std::endl;
    std::cout << "\n如果子类不能完全替换父类，那就不应该继承，" << std::endl;
    std::cout << "而应该使用组合或其他设计模式。" << std::endl;
}

int main() {
    std::cout << "=== 里氏替换原则演示程序 ===" << std::endl;
    std::cout << "Liskov Substitution Principle Demonstration" << std::endl;
    std::cout << "场景：嵌入式设备通信模块" << std::endl;
    
    try {
        // 演示坏设计
        demonstrateBadDesign();
        
        std::cout << "\n等待2秒..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // 演示好设计
        demonstrateGoodDesign();
        
        std::cout << "\n等待2秒..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // 演示替换
        demonstrateSubstitution();
        
        std::cout << "\n等待2秒..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // 对比分析
        demonstrateComparison();
        
        printSeparator("演示完成");
        std::cout << "\n【里氏替换原则核心思想】" << std::endl;
        std::cout << "子类对象必须能够替换所有父类对象，而不会破坏程序的正确性。" << std::endl;
        std::cout << "\n【关键要点】" << std::endl;
        std::cout << "1. 子类不能改变父类方法的契约" << std::endl;
        std::cout << "2. 子类不能抛出父类未声明的异常" << std::endl;
        std::cout << "3. 子类不能改变父类方法的前置条件或后置条件" << std::endl;
        std::cout << "4. 子类不能改变父类方法返回值的含义" << std::endl;
        std::cout << "5. 继承应该建立真正的『is-a』关系" << std::endl;
        std::cout << "\n【实际应用】" << std::endl;
        std::cout << "• 检查继承关系是否合理" << std::endl;
        std::cout << "• 定义明确的接口契约" << std::endl;
        std::cout << "• 编写契约测试（Contract Test）" << std::endl;
        std::cout << "• 遵循设计模式（如策略模式）" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "程序执行出错: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
