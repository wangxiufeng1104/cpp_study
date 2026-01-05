#include <iostream>
#include <memory>
#include <vector>
#include "bad_design.h"
#include "good_design.h"

void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << title << std::endl;
    std::cout << std::string(70, '=') << std::endl;
}

void demonstrateBadDesign() {
    printSeparator("坏设计演示 - 违反接口隔离原则");
    
    std::cout << "\n【场景1：传感器类】" << std::endl;
    {
        bad_design::TemperatureSensor sensor("sensor_001");
        
        // 尝试调用方法
        std::cout << "\n调用需要的方法：" << std::endl;
        double temp = sensor.readSensor();
        
        std::cout << "\n尝试调用不需要的方法：" << std::endl;
        try {
            sensor.executeCommand("start");
        } catch (const std::exception& e) {
            std::cout << "  " << e.what() << std::endl;
        }
        
        try {
            sensor.display("Hello");
        } catch (const std::exception& e) {
            std::cout << "  " << e.what() << std::endl;
        }
        
        try {
            sensor.connectNetwork("192.168.1.1", 8080);
        } catch (const std::exception& e) {
            std::cout << "  " << e.what() << std::endl;
        }
        
        try {
            sensor.sendNetworkData("test");
        } catch (const std::exception& e) {
            std::cout << "  " << e.what() << std::endl;
        }
        
        try {
            sensor.saveData("data");
        } catch (const std::exception& e) {
            std::cout << "  " << e.what() << std::endl;
        }
    }
    
    std::cout << "\n【场景2：执行器类】" << std::endl;
    {
        bad_design::MotorController motor("motor_001");
        
        std::cout << "\n调用需要的方法：" << std::endl;
        motor.executeCommand("start");
        motor.executeCommand("stop");
        
        std::cout << "\n尝试调用不需要的方法：" << std::endl;
        try {
            motor.readSensor();
        } catch (const std::exception& e) {
            std::cout << "  " << e.what() << std::endl;
        }
        
        try {
            motor.display("Status");
        } catch (const std::exception& e) {
            std::cout << "  " << e.what() << std::endl;
        }
    }
    
    std::cout << "\n【场景3：显示器类】" << std::endl;
    {
        bad_design::DisplayModule display("display_001");
        
        std::cout << "\n调用需要的方法：" << std::endl;
        display.display("Welcome");
        
        std::cout << "\n尝试调用不需要的方法：" << std::endl;
        try {
            display.readSensor();
        } catch (const std::exception& e) {
            std::cout << "  " << e.what() << std::endl;
        }
        
        try {
            display.executeCommand("cmd");
        } catch (const std::exception& e) {
            std::cout << "  " << e.what() << std::endl;
        }
    }
    
    std::cout << "\n【问题总结】" << std::endl;
    std::cout << "违反接口隔离原则的问题：" << std::endl;
    std::cout << "1. 接口臃肿：" << std::endl;
    std::cout << "   - IDevice接口包含所有可能的方法" << std::endl;
    std::cout << "   - 9个方法：传感器、执行器、显示器、网络、存储、日志" << std::endl;
    std::cout << "2. 被迫实现：" << std::endl;
    std::cout << "   - 传感器被迫实现8个不需要的方法" << std::endl;
    std::cout << "   - 执行器被迫实现8个不需要的方法" << std::endl;
    std::cout << "   - 显示器被迫实现8个不需要的方法" << std::endl;
    std::cout << "3. 空实现或异常：" << std::endl;
    std::cout << "   - 不需要的方法通常抛出异常" << std::endl;
    std::cout << "   - 增加了调用者的负担" << std::endl;
    std::cout << "4. 接口不清晰：" << std::endl;
    std::cout << "   - 接口职责不明，难以理解" << std::endl;
    std::cout << "   - 违反了单一职责原则" << std::endl;
    
    std::cout << "\n【后果】" << std::endl;
    std::cout << "✗ 代码复杂度高" << std::endl;
    std::cout << "✗ 难以维护和理解" << std::endl;
    std::cout << "✗ 虚函数调用开销" << std::endl;
    std::cout << "✗ 测试困难（需要mock很多方法）" << std::endl;
    std::cout << "✗ 客户端被迫依赖不使用的接口" << std::endl;
}

void demonstrateGoodDesign() {
    printSeparator("好设计演示 - 遵循接口隔离原则");
    
    std::cout << "\n【场景1：传感器类 - 只实现ISensor】" << std::endl;
    {
        good_design::TemperatureSensor sensor("sensor_001");
        
        std::cout << "\n调用需要的方法：" << std::endl;
        double temp = sensor.readValue();
        
        std::cout << "\n✓ 传感器只需实现ISensor接口（1个方法）" << std::endl;
        std::cout << "✓ 不需要实现其他接口" << std::endl;
        std::cout << "✓ 代码简洁清晰" << std::endl;
    }
    
    std::cout << "\n【场景2：执行器类 - 只实现IActuator】" << std::endl;
    {
        good_design::MotorController motor("motor_001");
        
        std::cout << "\n调用需要的方法：" << std::endl;
        motor.execute("start");
        motor.execute("stop");
        
        std::cout << "\n✓ 执行器只需实现IActuator接口（1个方法）" << std::endl;
        std::cout << "✓ 不需要实现其他接口" << std::endl;
        std::cout << "✓ 代码简洁清晰" << std::endl;
    }
    
    std::cout << "\n【场景3：显示器类 - 只实现IDisplay】" << std::endl;
    {
        good_design::DisplayModule display("display_001");
        
        std::cout << "\n调用需要的方法：" << std::endl;
        display.show("Welcome");
        display.show("Temperature: 25.5°C");
        
        std::cout << "\n✓ 显示器只需实现IDisplay接口（1个方法）" << std::endl;
        std::cout << "✓ 不需要实现其他接口" << std::endl;
        std::cout << "✓ 代码简洁清晰" << std::endl;
    }
    
    std::cout << "\n【场景4：网络类 - 只实现INetwork】" << std::endl;
    {
        good_design::WifiNetwork wifi("mywifi");
        
        std::cout << "\n调用需要的方法：" << std::endl;
        wifi.connect("192.168.1.100", 8080);
        wifi.send("{\"data\":\"test\"}");
        wifi.disconnect();
        
        std::cout << "\n✓ 网络只需实现INetwork接口（4个方法）" << std::endl;
        std::cout << "✓ 不需要实现其他接口" << std::endl;
        std::cout << "✓ 代码简洁清晰" << std::endl;
    }
    
    std::cout << "\n【场景5：存储类 - 只实现IStorage】" << std::endl;
    {
        good_design::FileStorage storage("data.txt");
        
        std::cout << "\n调用需要的方法：" << std::endl;
        storage.save("{\"temp\":25.5}");
        std::string data;
        storage.load(data);
        
        std::cout << "\n✓ 存储只需实现IStorage接口（2个方法）" << std::endl;
        std::cout << "✓ 不需要实现其他接口" << std::endl;
        std::cout << "✓ 代码简洁清晰" << std::endl;
    }
    
    std::cout << "\n【场景6：日志类 - 只实现ILogger】" << std::endl;
    {
        good_design::ConsoleLogger logger("app_logger");
        
        std::cout << "\n调用需要的方法：" << std::endl;
        logger.info("Application started");
        logger.error("Connection failed");
        
        std::cout << "\n✓ 日志只需实现ILogger接口（2个方法）" << std::endl;
        std::cout << "✓ 不需要实现其他接口" << std::endl;
        std::cout << "✓ 代码简洁清晰" << std::endl;
    }
    
    std::cout << "\n【场景7：智能设备 - 组合多个接口】" << std::endl;
    {
        good_design::SmartDevice device("smart_001");
        
        std::cout << "\n实现3个接口：ISensor、IActuator、IDisplay" << std::endl;
        double temp = device.readValue();
        device.execute("start");
        device.show("System Ready");
        
        std::cout << "\n✓ 可以按需组合接口" << std::endl;
        std::cout << "✓ 只实现真正需要的功能" << std::endl;
        std::cout << "✓ 灵活性高" << std::endl;
    }
    
    std::cout << "\n【场景8：网络化智能设备 - 组合4个接口】" << std::endl;
    {
        good_design::NetworkedSmartDevice device("smart_002");
        
        std::cout << "\n实现4个接口：ISensor、IActuator、INetwork、IStorage" << std::endl;
        double temp = device.readValue();
        device.execute("start");
        device.connect("cloud.server.com", 443);
        device.send("{\"temp\":27.0}");
        device.save("{\"timestamp\":\"2026-01-05\"}");
        
        std::cout << "\n✓ 可以按需组合接口" << std::endl;
        std::cout << "✓ 只实现真正需要的功能" << std::endl;
        std::cout << "✓ 灵活性高" << std::endl;
    }
    
    std::cout << "\n【优势总结】" << std::endl;
    std::cout << "遵循接口隔离原则的优势：" << std::endl;
    std::cout << "1. 接口简洁：" << std::endl;
    std::cout << "   - 每个接口职责单一" << std::endl;
    std::cout << "   - ISensor(1个方法)、IActuator(1个方法)、IDisplay(1个方法)" << std::endl;
    std::cout << "2. 按需实现：" << std::endl;
    std::cout << "   - 类只需实现需要的接口" << std::endl;
    std::cout << "   - 不需要实现不使用的方法" << std::endl;
    std::cout << "3. 代码清晰：" << std::endl;
    std::cout << "   - 每个类职责明确" << std::endl;
    std::cout << "   - 易于理解和维护" << std::endl;
    std::cout << "4. 灵活组合：" << std::endl;
    std::cout << "   - 复杂设备可以组合多个接口" << std::endl;
    std::cout << "   - 简单设备只需实现需要的接口" << std::endl;
    std::cout << "5. 易于测试：" << std::endl;
    std::cout << "   - 接口小而专注" << std::endl;
    std::cout << "   - Mock更容易" << std::endl;
}

void demonstrateInterfaceUsage() {
    printSeparator("接口使用演示");
    
    std::cout << "\n【使用细分的接口】" << std::endl;
    std::cout << "客户端只依赖真正需要的接口" << std::endl;
    
    std::cout << "\n场景1：只需要读取传感器" << std::endl;
    {
        void (*readSensorFunc)(good_design::ISensor*) = [](good_design::ISensor* sensor) {
            double value = sensor->readValue();
            std::cout << "  Read value: " << value << std::endl;
        };
        
        good_design::TemperatureSensor sensor("sensor_001");
        readSensorFunc(&sensor);
        
        std::cout << "  ✓ 只依赖ISensor接口" << std::endl;
    }
    
    std::cout << "\n场景2：只需要控制执行器" << std::endl;
    {
        void (*executeFunc)(good_design::IActuator*) = [](good_design::IActuator* actuator) {
            actuator->execute("start");
            actuator->execute("stop");
        };
        
        good_design::MotorController motor("motor_001");
        executeFunc(&motor);
        
        std::cout << "  ✓ 只依赖IActuator接口" << std::endl;
    }
    
    std::cout << "\n场景3：只需要显示信息" << std::endl;
    {
        void (*showFunc)(good_design::IDisplay*) = [](good_design::IDisplay* display) {
            display->show("Welcome");
            display->show("System Ready");
        };
        
        good_design::DisplayModule display("display_001");
        showFunc(&display);
        
        std::cout << "  ✓ 只依赖IDisplay接口" << std::endl;
    }
    
    std::cout << "\n【多态使用接口】" << std::endl;
    std::cout << "可以使用接口指针指向不同的实现" << std::endl;
    
    std::vector<std::unique_ptr<good_design::ISensor>> sensors;
    sensors.push_back(std::make_unique<good_design::TemperatureSensor>("sensor_001"));
    // 可以添加其他传感器实现
    // sensors.push_back(std::make_unique<HumiditySensor>("sensor_002"));
    
    std::cout << "\n遍历所有传感器：" << std::endl;
    for (auto& sensor : sensors) {
        double value = sensor->readValue();
        std::cout << "  Sensor value: " << value << std::endl;
    }
    
    std::cout << "\n【接口隔离的价值】" << std::endl;
    std::cout << "✓ 客户端只依赖需要的接口" << std::endl;
    std::cout << "✓ 接口小而专注" << std::endl;
    std::cout << "✓ 易于理解和维护" << std::endl;
    std::cout << "✓ 易于测试和Mock" << std::endl;
    std::cout << "✓ 支持灵活组合" << std::endl;
}

void demonstrateComparison() {
    printSeparator("设计对比分析");
    
    std::cout << "\n【违反ISP的后果】" << std::endl;
    std::cout << "1. 接口臃肿" << std::endl;
    std::cout << "   - 一个接口包含所有可能的方法" << std::endl;
    std::cout << "   - 违反单一职责原则" << std::endl;
    std::cout << "   - 难以理解和使用" << std::endl;
    
    std::cout << "\n2. 被迫实现" << std::endl;
    std::cout << "   - 客户端被迫实现不需要的方法" << std::endl;
    std::cout << "   - 导致空实现或异常抛出" << std::endl;
    std::cout << "   - 增加代码复杂度" << std::endl;
    
    std::cout << "\n3. 虚函数开销" << std::endl;
    std::cout << "   - 虚函数表增大" << std::endl;
    std::cout << "   - 调用开销增加" << std::endl;
    std::cout << "   - 影响性能" << std::endl;
    
    std::cout << "\n4. 依赖污染" << std::endl;
    std::cout << "   - 客户端依赖不使用的接口" << std::endl;
    std::cout << "   - 编译依赖增加" << std::endl;
    std::cout << "   - 重编译影响范围扩大" << std::endl;
    
    std::cout << "\n【遵循ISP的优势】" << std::endl;
    std::cout << "1. 接口简洁" << std::endl;
    std::cout << "   - 每个接口职责单一" << std::endl;
    std::cout << "   - 高内聚，低耦合" << std::endl;
    std::cout << "   - 易于理解" << std::endl;
    
    std::cout << "\n2. 按需实现" << std::endl;
    std::cout << "   - 类只实现需要的接口" << std::endl;
    std::cout << "   - 没有不必要的方法" << std::endl;
    std::cout << "   - 代码简洁" << std::endl;
    
    std::cout << "\n3. 易于维护" << std::endl;
    std::cout << "   - 修改接口影响范围小" << std::endl;
    std::cout << "   - 接口变化只影响相关类" << std::endl;
    std::cout << "   - 降低维护成本" << std::endl;
    
    std::cout << "\n4. 易于测试" << std::endl;
    std::cout << "   - 接口小而专注" << std::endl;
    std::cout << "   - Mock简单" << std::endl;
    std::cout << "   - 测试用例简洁" << std::endl;
    
    std::cout << "\n5. 灵活组合" << std::endl;
    std::cout << "   - 可以按需组合接口" << std::endl;
    std::cout << "   - 支持简单和复杂设备" << std::endl;
    std::cout << "   - 提高设计灵活性" << std::endl;
    
    std::cout << "\n【接口数量对比】" << std::endl;
    std::cout << "坏设计：" << std::endl;
    std::cout << "  - 1个大接口IDevice（9个方法）" << std::endl;
    std::cout << "  - 传感器：被迫实现9个方法（1个需要，8个不需要）" << std::endl;
    std::cout << "  - 执行器：被迫实现9个方法（1个需要，8个不需要）" << std::endl;
    std::cout << "  - 显示器：被迫实现9个方法（1个需要，8个不需要）" << std::endl;
    
    std::cout << "\n好设计：" << std::endl;
    std::cout << "  - 6个小接口" << std::endl;
    std::cout << "  - ISensor（1个方法）、IActuator（1个方法）、IDisplay（1个方法）" << std::endl;
    std::cout << "  - INetwork（4个方法）、IStorage（2个方法）、ILogger（2个方法）" << std::endl;
    std::cout << "  - 传感器：实现1个方法（正好需要）" << std::endl;
    std::cout << "  - 执行器：实现1个方法（正好需要）" << std::endl;
    std::cout << "  - 显示器：实现1个方法（正好需要）" << std::endl;
    
    std::cout << "\n【ISP与面向对象】" << std::endl;
    std::cout << "接口隔离原则是面向对象设计的核心之一" << std::endl;
    std::cout << "它帮助我们：" << std::endl;
    std::cout << "1. 设计职责清晰的接口" << std::endl;
    std::cout << "2. 避免臃肿的接口" << std::endl;
    std::cout << "3. 提高代码质量和可维护性" << std::endl;
    std::cout << "4. 增强系统的灵活性" << std::endl;
}

int main() {
    std::cout << "=== 接口隔离原则演示程序 ===" << std::endl;
    std::cout << "Interface Segregation Principle Demonstration" << std::endl;
    std::cout << "场景：嵌入式设备模块控制" << std::endl;
    
    // 演示坏设计
    demonstrateBadDesign();
    
    // 演示好设计
    demonstrateGoodDesign();
    
    // 演示接口使用
    demonstrateInterfaceUsage();
    
    // 对比分析
    demonstrateComparison();
    
    printSeparator("演示完成");
    std::cout << "\n【接口隔离原则核心思想】" << std::endl;
    std::cout << "客户端不应该被迫依赖它们不使用的接口。" << std::endl;
    std::cout << "\n【关键要点】" << std::endl;
    std::cout << "1. 接口应该小而专注" << std::endl;
    std::cout << "2. 每个接口应该有单一的职责" << std::endl;
    std::cout << "3. 客户端只依赖它们真正需要的接口" << std::endl;
    std::cout << "4. 接口应该高内聚、低耦合" << std::endl;
    std::cout << "5. 避免臃肿的接口" << std::endl;
    std::cout << "\n【实际应用】" << std::endl;
    std::cout << "• 设计细分的专用接口" << std::endl;
    std::cout << "• 按需组合接口" << std::endl;
    std::cout << "• 避免强迫客户端实现不需要的方法" << std::endl;
    std::cout << "• 定期审查接口设计" << std::endl;
    std::cout << "• 使用接口继承来组织相关方法" << std::endl;
    
    return 0;
}
