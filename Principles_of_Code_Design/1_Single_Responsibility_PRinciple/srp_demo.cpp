#include <iostream>
#include <cstdlib>
#include <ctime>
#include "bad_design.h"
#include "good_design.h"

void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << title << std::endl;
    std::cout << std::string(70, '=') << std::endl;
}

void demonstrateBadDesign() {
    printSeparator("坏设计演示 - 违反单一职责原则");
    
    std::cout << "\n【场景描述】" << std::endl;
    std::cout << "智能温湿度传感器设备，需要：" << std::endl;
    std::cout << "  1. 读取温湿度传感器数据（DHT11）" << std::endl;
    std::cout << "  2. 通过MQTT协议上报数据到云端" << std::endl;
    std::cout << "  3. 记录设备运行日志" << std::endl;
    
    std::cout << "\n【创建SensorDevice实例】" << std::endl;
    bad_design::SensorDevice device("sensor_bad_001");
    
    std::cout << "\n【运行设备控制循环（3次迭代）】" << std::endl;
    device.runControlLoop(3);
    
    std::cout << "\n" << std::string(70, '-') << std::endl;
    std::cout << "【坏设计的问题分析】" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    
    std::cout << "\n1. 单一类承担多个职责：" << std::endl;
    std::cout << "   - 职责1: 传感器数据读取" << std::endl;
    std::cout << "   - 职责2: MQTT通信" << std::endl;
    std::cout << "   - 职责3: 日志记录" << std::endl;
    std::cout << "   - 问题：一个类有3个引起变化的原因" << std::endl;
    
    std::cout << "\n2. 高耦合问题：" << std::endl;
    std::cout << "   - 传感器、MQTT、日志耦合在一起" << std::endl;
    std::cout << "   - 修改MQTT协议可能影响传感器读取" << std::endl;
    std::cout << "   - 更换传感器类型可能影响MQTT通信" << std::endl;
    std::cout << "   - 修改日志格式可能影响其他功能" << std::endl;
    
    std::cout << "\n3. 难以独立测试：" << std::endl;
    std::cout << "   - 测试MQTT功能时，传感器和日志也在运行" << std::endl;
    std::cout << "   - 无法隔离测试某个功能" << std::endl;
    std::cout << "   - 测试失败时难以定位问题" << std::endl;
    
    std::cout << "\n4. 代码复用性差：" << std::endl;
    std::cout << "   - 其他设备无法复用SensorDevice" << std::endl;
    std::cout << "   - 传感器读取逻辑无法单独使用" << std::endl;
    std::cout << "   - MQTT通信逻辑无法单独使用" << std::endl;
    
    std::cout << "\n5. 违反开闭原则：" << std::endl;
    std::cout << "   - 更换MQTT平台需要修改SensorDevice类" << std::endl;
    std::cout << "   - 更换传感器型号需要修改SensorDevice类" << std::endl;
    std::cout << "   - 添加HTTP协议支持需要修改SensorDevice类" << std::endl;
    
    std::cout << "\n6. 代码维护困难：" << std::endl;
    std::cout << "   - 类变得庞大且复杂" << std::endl;
    std::cout << "   - 理解代码需要知道所有功能" << std::endl;
    std::cout << "   - 修改任何功能都需要理解整个类" << std::endl;
}

void demonstrateGoodDesign() {
    printSeparator("好设计演示 - 遵循单一职责原则");
    
    std::cout << "\n【场景描述】" << std::endl;
    std::cout << "智能温湿度传感器设备，需要：" << std::endl;
    std::cout << "  1. 读取温湿度传感器数据（DHT11）" << std::endl;
    std::cout << "  2. 通过MQTT协议上报数据到云端" << std::endl;
    std::cout << "  3. 记录设备运行日志" << std::endl;
    
    std::cout << "\n【创建职责分离的组件】" << std::endl;
    std::cout << "  - SensorReader: 只负责传感器数据读取" << std::endl;
    std::cout << "  - MqttPublisher: 只负责MQTT数据上报" << std::endl;
    std::cout << "  - DeviceLogger: 只负责日志记录" << std::endl;
    std::cout << "  - SensorController: 只负责业务协调" << std::endl;
    
    std::cout << "\n【创建SensorController实例】" << std::endl;
    auto controller = std::make_unique<good_design::SensorController>(
        std::make_unique<good_design::DHT11SensorReader>("sensor_good_001"),
        std::make_unique<good_design::StandardMqttPublisher>("mqtt://localhost:1883", "client_001"),
        std::make_unique<good_design::FileDeviceLogger>("device.log"),
        "sensor_good_001"
    );
    
    std::cout << "\n【运行设备控制循环（3次迭代）】" << std::endl;
    controller->initialize();
    controller->runControlLoop(3);
    controller->shutdown();
    
    std::cout << "\n" << std::string(70, '-') << std::endl;
    std::cout << "【好设计的优势分析】" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    
    std::cout << "\n1. 单一职责原则：" << std::endl;
    std::cout << "   - SensorReader: 只负责传感器数据读取" << std::endl;
    std::cout << "   - MqttPublisher: 只负责MQTT数据上报" << std::endl;
    std::cout << "   - DeviceLogger: 只负责日志记录" << std::endl;
    std::cout << "   - SensorController: 只负责业务协调" << std::endl;
    std::cout << "   - 优势：每个类只有一个引起变化的原因" << std::endl;
    
    std::cout << "\n2. 低耦合高内聚：" << std::endl;
    std::cout << "   - 各组件通过接口解耦" << std::endl;
    std::cout << "   - 修改MQTT协议不影响传感器读取" << std::endl;
    std::cout << "   - 更换传感器类型不影响MQTT通信" << std::endl;
    std::cout << "   - 修改日志格式不影响其他功能" << std::endl;
    
    std::cout << "\n3. 易于独立测试：" << std::endl;
    std::cout << "   - 可以独立测试MQTT发布功能" << std::endl;
    std::cout << "   - 可以独立测试传感器读取功能" << std::endl;
    std::cout << "   - 可以独立测试日志记录功能" << std::endl;
    std::cout << "   - 测试失败时可以准确定位问题" << std::endl;
    
    std::cout << "\n4. 高代码复用性：" << std::endl;
    std::cout << "   - SensorReader可以在其他设备中复用" << std::endl;
    std::cout << "   - MqttPublisher可以在其他设备中复用" << std::endl;
    std::cout << "   - DeviceLogger可以作为通用日志组件" << std::endl;
    std::cout << "   - 组件可以在不同设备间复用" << std::endl;
    
    std::cout << "\n5. 符合开闭原则：" << std::endl;
    std::cout << "   - 更换MQTT平台只需创建新的Publisher类" << std::endl;
    std::cout << "   - 更换传感器只需创建新的Reader类" << std::endl;
    std::cout << "   - 添加HTTP协议支持只需创建HttpPublisher类" << std::endl;
    std::cout << "   - 扩展时无需修改现有代码" << std::endl;
    
    std::cout << "\n6. 易于维护和扩展：" << std::endl;
    std::cout << "   - 每个类小而专注，易于理解" << std::endl;
    std::cout << "   - 修改功能只需关注对应的类" << std::endl;
    std::cout << "   - 新人容易理解代码结构" << std::endl;
    std::cout << "   - 支持运行时动态替换组件" << std::endl;
}

void demonstrateExtension() {
    printSeparator("扩展性演示 - 展示好设计的扩展能力");
    
    std::cout << "\n【场景1: 更换MQTT平台（从标准MQTT到阿里云IoT）】" << std::endl;
    auto controller = std::make_unique<good_design::SensorController>(
        std::make_unique<good_design::DHT11SensorReader>("sensor_aliyun"),
        std::make_unique<good_design::StandardMqttPublisher>("mqtt://localhost:1883", "client_001"),
        std::make_unique<good_design::ConsoleDeviceLogger>(),
        "sensor_aliyun"
    );
    
    controller->initialize();
    std::cout << "\n【运行时替换MQTT发布器为阿里云IoT】" << std::endl;
    controller->setMqttPublisher(
        std::make_unique<good_design::AliyunMqttPublisher>("product_001", "device_001")
    );
    controller->runControlLoop(2);
    controller->shutdown();
    
    std::cout << "\n【场景2: 更换传感器型号（从DHT11到SHT30）】" << std::endl;
    controller = std::make_unique<good_design::SensorController>(
        std::make_unique<good_design::DHT11SensorReader>("sensor_sht30"),
        std::make_unique<good_design::StandardMqttPublisher>("mqtt://localhost:1883", "client_002"),
        std::make_unique<good_design::ConsoleDeviceLogger>(),
        "sensor_sht30"
    );
    
    controller->initialize();
    std::cout << "\n【运行时替换传感器为SHT30（I2C接口）】" << std::endl;
    controller->setSensorReader(
        std::make_unique<good_design::SHT30SensorReader>("sensor_sht30")
    );
    controller->runControlLoop(2);
    controller->shutdown();
    
    std::cout << "\n【场景3: 添加HTTP协议支持】" << std::endl;
    controller = std::make_unique<good_design::SensorController>(
        std::make_unique<good_design::DHT11SensorReader>("sensor_http"),
        std::make_unique<good_design::HttpPublisher>("http://localhost:8080/api/sensor"),
        std::make_unique<good_design::ConsoleDeviceLogger>(),
        "sensor_http"
    );
    
    controller->initialize();
    std::cout << "\n【使用HTTP协议上报数据】" << std::endl;
    controller->runControlLoop(2);
    controller->shutdown();
}

void demonstrateComparison() {
    printSeparator("设计对比分析");
    
    std::cout << "\n【嵌入式设备场景对比】" << std::endl;
    std::cout << "\n坏设计 (SensorDevice):" << std::endl;
    std::cout << "  - 1个类，约150行代码" << std::endl;
    std::cout << "  - 承担3个不同职责（传感器、MQTT、日志）" << std::endl;
    std::cout << "  - 高耦合，难以维护" << std::endl;
    std::cout << "  - 难以独立测试和扩展" << std::endl;
    std::cout << "  - 更换MQTT或传感器需要修改主类" << std::endl;
    std::cout << "  - 添加新协议需要修改主类" << std::endl;
    
    std::cout << "\n好设计 (SensorController + 组件):" << std::endl;
    std::cout << "  - 4个类（Reader、Publisher、Logger、Controller）" << std::endl;
    std::cout << "  - 每个类职责单一" << std::endl;
    std::cout << "  - 低耦合，易于维护" << std::endl;
    std::cout << "  - 可以独立测试每个组件" << std::endl;
    std::cout << "  - 更换MQTT只需创建新的Publisher" << std::endl;
    std::cout << "  - 更换传感器只需创建新的Reader" << std::endl;
    std::cout << "  - 添加新协议只需创建新的Publisher" << std::endl;
    std::cout << "  - 支持运行时动态替换组件" << std::endl;
    
    std::cout << "\n【实际应用场景举例】" << std::endl;
    
    std::cout << "\n场景1: 更换MQTT Broker" << std::endl;
    std::cout << "  坏设计: 需要修改SensorDevice类，可能影响传感器读取和日志" << std::endl;
    std::cout << "  好设计: 只需替换MqttPublisher组件，其他组件不受影响" << std::endl;
    std::cout << "         可以创建AliyunMqttPublisher、AwsMqttPublisher等" << std::endl;
    
    std::cout << "\n场景2: 更换温湿度传感器型号" << std::endl;
    std::cout << "  坏设计: 需要修改SensorDevice类，可能影响MQTT通信和日志" << std::endl;
    std::cout << "  好设计: 只需修改SensorReader类，其他组件不受影响" << std::endl;
    std::cout << "         可以创建DHT11SensorReader、SHT30SensorReader等" << std::endl;
    
    std::cout << "\n场景3: 添加HTTP协议支持" << std::endl;
    std::cout << "  坏设计: 需要在SensorDevice类中添加HTTP相关代码" << std::endl;
    std::cout << "         类变得更加臃肿，职责混乱" << std::endl;
    std::cout << "  好设计: 只需创建HttpPublisher组件" << std::endl;
    std::cout << "         可以与MqttPublisher并存，灵活切换" << std::endl;
    
    std::cout << "\n场景4: 单元测试" << std::endl;
    std::cout << "  坏设计: 测试MQTT时必须包含传感器和日志功能" << std::endl;
    std::cout << "         难以隔离测试，测试失败时难以定位问题" << std::endl;
    std::cout << "  好设计: 可以独立测试MqttPublisher" << std::endl;
    std::cout << "         可以mock MQTT broker，测试速度快且准确" << std::endl;
    std::cout << "         测试失败时明确是哪个组件的问题" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "单一职责原则演示" << std::endl;
    std::cout << "Single Responsibility Principle" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "场景：嵌入式Linux设备（智能家居）" << std::endl;
    
    // 初始化随机数种子
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    
    try {
        // 演示坏设计
        demonstrateBadDesign();
        
        std::cout << "\n\n等待2秒..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // 演示好设计
        demonstrateGoodDesign();
        
        std::cout << "\n\n等待2秒..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // 演示扩展性
        demonstrateExtension();
        
        std::cout << "\n\n等待2秒..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // 对比分析
        demonstrateComparison();
        
        printSeparator("演示完成");
        
        std::cout << "\n【单一职责原则核心思想】" << std::endl;
        std::cout << "一个类应该有且仅有一个引起它变化的原因。" << std::endl;
        std::cout << "换句话说，一个类应该只负责一项职责。" << std::endl;
        
        std::cout << "\n【在嵌入式开发中的重要性】" << std::endl;
        std::cout << "\n嵌入式设备的特点:" << std::endl;
        std::cout << "1. 资源受限（内存、CPU）" << std::endl;
        std::cout << "2. 硬件依赖性强" << std::endl;
        std::cout << "3. 协议和接口经常变化" << std::endl;
        std::cout << "4. 需要长期稳定运行" << std::endl;
        std::cout << "5. 支持远程升级和维护" << std::endl;
        
        std::cout << "\n应用单一职责原则的好处:" << std::endl;
        std::cout << "1. 降低硬件变更的影响范围" << std::endl;
        std::cout << "2. 便于模块化测试和调试" << std::endl;
        std::cout << "3. 提高代码的可维护性和可扩展性" << std::endl;
        std::cout << "4. 支持组件复用，减少重复开发" << std::endl;
        std::cout << "5. 符合开闭原则，对扩展开放" << std::endl;
        
        std::cout << "\n【总结】" << std::endl;
        std::cout << "单一职责原则帮助我们：" << std::endl;
        std::cout << "  - 构建清晰的系统架构" << std::endl;
        std::cout << "  - 提高代码质量和可维护性" << std::endl;
        std::cout << "  - 降低系统复杂性" << std::endl;
        std::cout << "  - 增强代码的复用性和测试性" << std::endl;
        
        std::cout << "\n记住：一个类做好一件事，比做所有事但都做不好要强得多。" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "\n程序执行出错: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
