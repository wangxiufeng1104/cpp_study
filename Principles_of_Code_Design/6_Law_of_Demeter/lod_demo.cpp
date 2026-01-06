#include <iostream>
#include <memory>
#include "bad_design.h"
#include "good_design.h"

void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << title << std::endl;
    std::cout << std::string(70, '=') << std::endl;
}

void demonstrateBadDesign() {
    printSeparator("坏设计演示 - 违反迪米特法则");
    
    std::cout << "\n【场景1：创建设备和监控系统】" << std::endl;
    bad_design::DeviceController controller("device_001", "https://api.example.com");
    controller.addSensor("temp_001", "temperature");
    controller.addSensor("humidity_001", "humidity");
    controller.addSensor("pressure_001", "pressure");
    
    bad_design::Monitor monitor("monitor_001", &controller);
    
    std::cout << "\n【场景2：报告单个传感器数据】" << std::endl;
    monitor.reportSingleSensorData(0);
    
    std::cout << "\n【场景3：报告所有传感器数据】" << std::endl;
    monitor.reportAllSensorData();
    
    std::cout << "\n【场景4：报告特定传感器值】" << std::endl;
    monitor.reportSpecificSensorValue(1);
    
    std::cout << "\n【场景5：处理设备数据】" << std::endl;
    monitor.processDeviceData();
    
    std::cout << "\n【问题分析】" << std::endl;
    std::cout << "违反迪米特法则的问题：" << std::endl;
    std::cout << "1. Monitor直接访问'陌生人'对象" << std::endl;
    std::cout << "   - 访问DeviceController内部的Sensor对象" << std::endl;
    std::cout << "   - 访问DeviceController内部的Network对象" << std::endl;
    std::cout << "   - 这些对象不是Monitor的直接朋友" << std::endl;
    
    std::cout << "\n2. Monitor需要了解DeviceController的内部结构" << std::endl;
    std::cout << "   - 知道DeviceController包含Sensor" << std::endl;
    std::cout << "   - 知道DeviceController包含Network" << std::endl;
    std::cout << "   - 知道如何获取和操作这些对象" << std::endl;
    
    std::cout << "\n3. 高耦合，修改影响范围大" << std::endl;
    std::cout << "   - 修改DeviceController内部结构需要修改Monitor" << std::endl;
    std::cout << "   - 修改Sensor接口需要修改Monitor" << std::endl;
    std::cout << "   - 修改Network接口需要修改Monitor" << std::endl;
    
    std::cout << "\n4. 违反最少知识原则" << std::endl;
    std::cout << "   - Monitor知道太多不应该知道的信息" << std::endl;
    std::cout << "   - Monitor承担了不属于自己的职责" << std::endl;
    
    std::cout << "\n【具体问题】" << std::endl;
    std::cout << "问题链：" << std::endl;
    std::cout << "  Monitor -> DeviceController.getSensor()" << std::endl;
    std::cout << "         -> Sensor.getData()           (访问陌生人)" << std::endl;
    std::cout << "  Monitor -> DeviceController.getNetwork()" << std::endl;
    std::cout << "         -> Network.send()            (访问陌生人)" << std::endl;
    
    std::cout << "\n【后果】" << std::endl;
    std::cout << "✗ 耦合度高（Monitor与多个对象耦合）" << std::endl;
    std::cout << "✗ 难以维护（修改影响范围大）" << std::endl;
    std::cout << "✗ 难以测试（需要模拟多个对象）" << std::endl;
    std::cout << "✗ 违反封装（暴露内部实现）" << std::endl;
    std::cout << "✗ 代码复杂（调用链过长）" << std::endl;
}

void demonstrateGoodDesign() {
    printSeparator("好设计演示 - 遵循迪米特法则");
    
    std::cout << "\n【场景1：创建设备和监控系统】" << std::endl;
    good_design::DeviceController controller("device_001", "https://api.example.com");
    controller.addSensor("temp_001", "temperature");
    controller.addSensor("humidity_001", "humidity");
    controller.addSensor("pressure_001", "pressure");
    
    good_design::Monitor monitor("monitor_001", &controller);
    
    std::cout << "\n【场景2：报告单个传感器数据】" << std::endl;
    monitor.reportSingleSensorData(0);
    
    std::cout << "\n【场景3：报告所有传感器数据】" << std::endl;
    monitor.reportAllSensorData();
    
    std::cout << "\n【场景4：报告特定传感器值】" << std::endl;
    monitor.reportSpecificSensorValue(1);
    
    std::cout << "\n【场景5：处理设备数据】" << std::endl;
    monitor.processDeviceData();
    
    std::cout << "\n【场景6：更新并报告传感器】" << std::endl;
    monitor.updateAndReportSensor(2, 1013.25);
    
    std::cout << "\n【场景7：监控所有传感器】" << std::endl;
    monitor.monitorAllSensors();
    
    std::cout << "\n【优势分析】" << std::endl;
    std::cout << "遵循迪米特法则的优势：" << std::endl;
    std::cout << "1. Monitor只与直接朋友通信" << std::endl;
    std::cout << "   - 只调用DeviceController的方法" << std::endl;
    std::cout << "   - 不访问DeviceController内部的Sensor" << std::endl;
    std::cout << "   - 不访问DeviceController内部的Network" << std::endl;
    
    std::cout << "\n2. Monitor不需要了解DeviceController的内部结构" << std::endl;
    std::cout << "   - 不需要知道DeviceController如何存储传感器" << std::endl;
    std::cout << "   - 不需要知道DeviceController如何实现网络" << std::endl;
    std::cout << "   - 只需要知道DeviceController提供的接口" << std::endl;
    
    std::cout << "\n3. 低耦合，修改影响范围小" << std::endl;
    std::cout << "   - 修改DeviceController内部结构不影响Monitor" << std::endl;
    std::cout << "   - 修改Sensor接口不影响Monitor" << std::endl;
    std::cout << "   - 修改Network接口不影响Monitor" << std::endl;
    
    std::cout << "\n4. 符合最少知识原则" << std::endl;
    std::cout << "   - Monitor只知道自己需要知道的" << std::endl;
    std::cout << "   - Monitor只承担自己的职责" << std::endl;
    
    std::cout << "\n【具体优势】" << std::endl;
    std::cout << "调用链：" << std::endl;
    std::cout << "  Monitor -> DeviceController.reportSensorData()" << std::endl;
    std::cout << "  Monitor -> DeviceController.reportAllSensorData()" << std::endl;
    std::cout << "  Monitor -> DeviceController.getSensorValue()" << std::endl;
    
    std::cout << "\n【优势总结】" << std::endl;
    std::cout << "✓ 低耦合（Monitor只与DeviceController耦合）" << std::endl;
    std::cout << "✓ 易于维护（修改影响范围小）" << std::endl;
    std::cout << "✓ 易于测试（只需模拟DeviceController）" << std::endl;
    std::cout << "✓ 遵循封装（隐藏内部实现）" << std::endl;
    std::cout << "✓ 代码清晰（调用链短）" << std::endl;
    std::cout << "✓ 职责清晰（每个类职责明确）" << std::endl;
}

void demonstratePrinciple() {
    printSeparator("迪米特法则核心思想");
    
    std::cout << "\n【迪米特法则定义】" << std::endl;
    std::cout << "一个对象应该对其他对象有尽可能少的了解。" << std::endl;
    std::cout << "只与你的'直接朋友'通信，不要跟'陌生人'说话。" << std::endl;
    
    std::cout << "\n【谁是'直接朋友'】" << std::endl;
    std::cout << "对于对象M，以下对象是'直接朋友'：" << std::endl;
    std::cout << "1. M本身（this）" << std::endl;
    std::cout << "2. M的成员变量" << std::endl;
    std::cout << "3. M的方法参数" << std::endl;
    std::cout << "4. M创建的对象" << std::endl;
    
    std::cout << "\n【谁是'陌生人'】" << std::endl;
    std::cout << "以下对象是'陌生人'：" << std::endl;
    std::cout << "1. '直接朋友'的内部对象" << std::endl;
    std::cout << "2. 方法返回的对象（除非是'直接朋友'）" << std::endl;
    std::cout << "3. 方法参数的内部对象" << std::endl;
    
    std::cout << "\n【示例分析】" << std::endl;
    std::cout << "坏设计：" << std::endl;
    std::cout << "  Monitor: reportSingleSensorData()" << std::endl;
    std::cout << "    controller->getSensor(index)  (返回Sensor，是陌生人)" << std::endl;
    std::cout << "    sensor->getData()              (与陌生人通信)" << std::endl;
    std::cout << "    controller->getNetwork()      (返回Network，是陌生人)" << std::endl;
    std::cout << "    network->send(data)           (与陌生人通信)" << std::endl;
    
    std::cout << "\n好设计：" << std::endl;
    std::cout << "  Monitor: reportSingleSensorData()" << std::endl;
    std::cout << "    controller->reportSensorData(index)  (只与直接朋友通信)" << std::endl;
    
    std::cout << "\n【最少知识原则（Principle of Least Knowledge）】" << std::endl;
    std::cout << "迪米特法则也被称为'最少知识原则'：" << std::endl;
    std::cout << "1. 每个对象应该只了解与自己相关的对象" << std::endl;
    std::cout << "2. 不应该知道系统中其他对象的存在" << std::endl;
    std::cout << "3. 只通过'直接朋友'与其他对象交互" << std::endl;
    
    std::cout << "\n【迪米特法则的好处】" << std::endl;
    std::cout << "1. 降低耦合度" << std::endl;
    std::cout << "   - 减少类之间的依赖关系" << std::endl;
    std::cout << "   - 降低修改影响的风险" << std::endl;
    std::cout << "   - 提高系统的稳定性" << std::endl;
    
    std::cout << "\n2. 提高可维护性" << std::endl;
    std::cout << "   - 修改内部实现不影响外部" << std::endl;
    std::cout << "   - 降低代码维护成本" << std::endl;
    std::cout << "   - 便于重构和优化" << std::endl;
    
    std::cout << "\n3. 增强可测试性" << std::endl;
    std::cout << "   - 只需模拟'直接朋友'" << std::endl;
    std::cout << "   - 减少测试复杂度" << std::endl;
    std::cout << "   - 提高测试覆盖率" << std::endl;
    
    std::cout << "\n4. 提高代码质量" << std::endl;
    std::cout << "   - 代码结构更清晰" << std::endl;
    std::cout << "   - 职责划分更明确" << std::endl;
    std::cout << "   - 便于理解和扩展" << std::endl;
}

void demonstrateComparison() {
    printSeparator("设计对比分析");
    
    std::cout << "\n【调用链对比】" << std::endl;
    std::cout << "坏设计（违反LoD）：" << std::endl;
    std::cout << "  Monitor" << std::endl;
    std::cout << "    -> controller.getSensor()" << std::endl;
    std::cout << "    -> sensor.getData()             (陌生人)" << std::endl;
    std::cout << "    -> controller.getNetwork()" << std::endl;
    std::cout << "    -> network.send(data)          (陌生人)" << std::endl;
    
    std::cout << "\n好设计（遵循LoD）：" << std::endl;
    std::cout << "  Monitor" << std::endl;
    std::cout << "    -> controller.reportSensorData()" << std::endl;
    
    std::cout << "\n【依赖关系对比】" << std::endl;
    std::cout << "坏设计：" << std::endl;
    std::cout << "  Monitor -> DeviceController (直接朋友)" << std::endl;
    std::cout << "  Monitor -> Sensor         (陌生人)" << std::endl;
    std::cout << "  Monitor -> Network        (陌生人)" << std::endl;
    
    std::cout << "\n好设计：" << std::endl;
    std::cout << "  Monitor -> DeviceController (直接朋友)" << std::endl;
    
    std::cout << "\n【可维护性对比】" << std::endl;
    std::cout << "坏设计:" << std::endl;
    std::cout << "  - 修改Sensor接口需要修改Monitor" << std::endl;
    std::cout << "  - 修改Network接口需要修改Monitor" << std::endl;
    std::cout << "  - 修改DeviceController内部需要修改Monitor" << std::endl;
    std::cout << "  - 影响范围大，维护成本高" << std::endl;
    
    std::cout << "\n好设计:" << std::endl;
    std::cout << "  - 修改Sensor接口不影响Monitor" << std::endl;
    std::cout << "  - 修改Network接口不影响Monitor" << std::endl;
    std::cout << "  - 修改DeviceController内部不影响Monitor" << std::endl;
    std::cout << "  - 影响范围小，维护成本低" << std::endl;
    
    std::cout << "\n【可测试性对比】" << std::endl;
    std::cout << "坏设计:" << std::endl;
    std::cout << "  - 需要模拟DeviceController" << std::endl;
    std::cout << "  - 需要模拟Sensor" << std::endl;
    std::cout << "  - 需要模拟Network" << std::endl;
    std::cout << "  - 测试复杂度高" << std::endl;
    
    std::cout << "\n好设计:" << std::endl;
    std::cout << "  - 只需模拟DeviceController" << std::endl;
    std::cout << "  - 不需要模拟Sensor" << std::endl;
    std::cout << "  - 不需要模拟Network" << std::endl;
    std::cout << "  - 测试复杂度低" << std::endl;
}

int main() {
    std::cout << "=== 迪米特法则演示程序 ===" << std::endl;
    std::cout << "Law of Demeter Demonstration" << std::endl;
    std::cout << "场景：嵌入式设备监控系统" << std::endl;
    
    try {
        // 演示坏设计
        demonstrateBadDesign();
        
        // 演示好设计
        demonstrateGoodDesign();
        
        // 演示迪米特法则核心思想
        demonstratePrinciple();
        
        // 对比分析
        demonstrateComparison();
        
        printSeparator("演示完成");
        std::cout << "\n【迪米特法则核心思想】" << std::endl;
        std::cout << "只与你的'直接朋友'通信，不要跟'陌生人'说话。" << std::endl;
        
        std::cout << "\n【关键要点】" << std::endl;
        std::cout << "1. 识别'直接朋友'：成员变量、方法参数、创建的对象" << std::endl;
        std::cout << "2. 避免访问'陌生人'：不要深入访问间接对象" << std::endl;
        std::cout << "3. 提供高层接口：让'直接朋友'封装复杂逻辑" << std::endl;
        std::cout << "4. 最少知识原则：每个对象只了解自己需要的" << std::endl;
        std::cout << "5. 降低耦合：减少不必要的依赖关系" << std::endl;
        
        std::cout << "\n【实际应用】" << std::endl;
        std::cout << "• 嵌入式系统设计" << std::endl;
        std::cout << "• 微服务架构" << std::endl;
        std::cout << "• 分布式系统" << std::endl;
        std::cout << "• 插件系统" << std::endl;
        std::cout << "• API设计" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "程序执行出错: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
