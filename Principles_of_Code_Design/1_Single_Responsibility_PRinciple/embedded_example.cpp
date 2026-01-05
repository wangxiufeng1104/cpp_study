#include <iostream>
#include <cstdlib>
#include "embedded_example.h"

void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void demonstrateBadDesign() {
    printSeparator("坏设计演示 - 智能传感器设备");
    
    std::cout << "\n创建SensorDevice实例..." << std::endl;
    bad_design::SensorDevice device;
    
    std::cout << "\n运行设备控制循环（5次迭代）..." << std::endl;
    device.runControlLoop(5);
    
    std::cout << "\n坏设计的问题分析:" << std::endl;
    std::cout << "1. SensorDevice类承担了3个不同的职责" << std::endl;
    std::cout << "2. 传感器读取、MQTT通信、日志记录耦合在一起" << std::endl;
    std::cout << "3. 修改MQTT协议需要修改SensorDevice类" << std::endl;
    std::cout << "4. 更换传感器类型需要修改SensorDevice类" << std::endl;
    std::cout << "5. 难以独立测试各个功能模块" << std::endl;
    std::cout << "6. 代码复用性差，其他设备无法复用组件" << std::endl;
}

void demonstrateGoodDesign() {
    printSeparator("好设计演示 - 智能传感器设备");
    
    std::cout << "\n创建各职责分离的组件..." << std::endl;
    good_design::SensorController controller;
    
    std::cout << "\n运行设备控制循环（5次迭代）..." << std::endl;
    controller.runControlLoop(5);
    
    std::cout << "\n好设计的优势:" << std::endl;
    std::cout << "1. 每个类只负责一个职责" << std::endl;
    std::cout << "2. SensorReader只负责传感器数据采集" << std::endl;
    std::cout << "3. MqttPublisher只负责数据上报" << std::endl;
    std::cout << "4. DeviceLogger只负责日志记录" << std::endl;
    std::cout << "5. SensorController只负责业务协调" << std::endl;
    std::cout << "6. 组件之间松耦合，可以独立替换" << std::endl;
    std::cout << "7. 修改MQTT协议只需替换MqttPublisher" << std::endl;
    std::cout << "8. 更换传感器只需修改SensorReader" << std::endl;
    std::cout << "9. 每个组件可以独立测试" << std::endl;
    std::cout << "10. 代码复用性高，其他设备可以复用组件" << std::endl;
}

void demonstrateComparison() {
    printSeparator("设计对比分析");
    
    std::cout << "\n【嵌入式设备场景对比】" << std::endl;
    std::cout << "\n坏设计 (SensorDevice):" << std::endl;
    std::cout << "  - 1个类，约80行代码" << std::endl;
    std::cout << "  - 承担3个不同职责" << std::endl;
    std::cout << "  - 高耦合，难以维护" << std::endl;
    std::cout << "  - 难以测试和扩展" << std::endl;
    
    std::cout << "\n好设计 (SensorController + 组件):" << std::endl;
    std::cout << "  - 4个类，每个类职责单一" << std::endl;
    std::cout << "  - 低耦合，易于维护" << std::endl;
    std::cout << "  - 可以独立测试每个组件" << std::endl;
    std::cout << "  - 支持灵活组合和扩展" << std::endl;
    
    std::cout << "\n【实际应用场景举例】" << std::endl;
    
    std::cout << "\n场景1: 更换MQTT Broker" << std::endl;
    std::cout << "  - 坏设计: 需要修改SensorDevice类，可能影响传感器读取" << std::endl;
    std::cout << "  - 好设计: 只需替换MqttPublisher组件，其他组件不受影响" << std::endl;
    
    std::cout << "\n场景2: 更换温湿度传感器型号" << std::endl;
    std::cout << "  - 坏设计: 需要修改SensorDevice类，可能影响MQTT通信" << std::endl;
    std::cout << "  - 好设计: 只需修改SensorReader类，其他组件不受影响" << std::endl;
    
    std::cout << "\n场景3: 添加HTTP协议支持" << std::endl;
    std::cout << "  - 坏设计: 需要在SensorDevice类中添加HTTP相关代码" << std::endl;
    std::cout << "  - 好设计: 只需创建HttpPublisher组件，与MqttPublisher并存" << std::endl;
    
    std::cout << "\n场景4: 单元测试" << std::endl;
    std::cout << "  - 坏设计: 测试MQTT时必须包含传感器和日志功能" << std::endl;
    std::cout << "  - 好设计: 可以独立测试MqttPublisher，无需其他组件" << std::endl;
}

int main() {
    std::cout << "=== 单一职责原则演示 - 智能家居设备 ===" << std::endl;
    std::cout << "Single Responsibility Principle - Smart Home Device" << std::endl;
    
    // 初始化随机数种子
    std::srand(static_cast<unsigned>(time(nullptr)));
    
    try {
        // 演示坏设计
        demonstrateBadDesign();
        
        std::cout << "\n\n等待2秒..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // 演示好设计
        demonstrateGoodDesign();
        
        // 对比分析
        demonstrateComparison();
        
        printSeparator("演示完成");
        std::cout << "\n【单一职责原则在嵌入式开发中的重要性】" << std::endl;
        std::cout << "\n嵌入式设备的特点:" << std::endl;
        std::cout << "1. 资源受限（内存、CPU）" << std::endl;
        std::cout << "2. 硬件依赖性强" << std::endl;
        std::cout << "3. 协议和接口经常变化" << std::endl;
        std::cout << "4. 需要长期稳定运行" << std::endl;
        
        std::cout << "\n应用单一职责原则的好处:" << std::endl;
        std::cout << "1. 降低硬件变更的影响范围" << std::endl;
        std::cout << "2. 便于模块化测试和调试" << std::endl;
        std::cout << "3. 提高代码的可维护性和可扩展性" << std::endl;
        std::cout << "4. 支持组件复用，减少重复开发" << std::endl;
        
        std::cout << "\n核心思想:" << std::endl;
        std::cout << "一个类应该有且仅有一个引起它变化的原因。" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "程序执行出错: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
