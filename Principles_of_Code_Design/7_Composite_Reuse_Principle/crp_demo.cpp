#include <iostream>
#include "bad_design.h"
#include "good_design.h"

void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << title << std::endl;
    std::cout << std::string(70, '=') << std::endl;
}

void demonstrateBadDesign() {
    printSeparator("坏设计演示 - 使用继承复用（违反组合复用原则）");
    
    std::cout << "\n【场景1：创建智能设备（继承所有能力）】" << std::endl;
    bad_design::SmartDevice smartDevice(
        "smart_001",
        "WiFi",
        "Flash",
        "/data/smart.txt",
        "Temperature"
    );
    smartDevice.sendData();
    smartDevice.saveData();
    
    std::cout << "\n【场景2：创建工业设备（继承部分能力）】" << std::endl;
    bad_design::IndustrialDevice industrialDevice(
        "industrial_001",
        "Ethernet",
        "Humidity"
    );
    industrialDevice.sendData();
    
    std::cout << "\n【场景3：创建IoT设备（继承部分能力）】" << std::endl;
    bad_design::IoTDevice iotDevice(
        "iot_001",
        "SD",
        "/mnt/sd/iot.txt",
        "Motion"
    );
    iotDevice.saveData();
    
    std::cout << "\n【场景4：创建网络存储设备（需要新建类）】" << std::endl;
    bad_design::NetworkStorageDevice networkStorageDevice(
        "netstorage_001",
        "5G",
        "eMMC",
        "/data/netstorage.txt"
    );
    networkStorageDevice.transferData();
    
    std::cout << "\n【问题分析】" << std::endl;
    std::cout << "使用继承复用的问题：" << std::endl;
    std::cout << "1. 类爆炸" << std::endl;
    std::cout << "   - 每种能力组合都需要创建新类" << std::endl;
    std::cout << "   - SmartDevice、IndustrialDevice、IoTDevice、NetworkStorageDevice..." << std::endl;
    
    std::cout << "\n2. 继承在编译时确定" << std::endl;
    std::cout << "   - 运行时无法改变能力组合" << std::endl;
    std::cout << "   - 无法动态添加或移除能力" << std::endl;
    
    std::cout << "\n3. 紧耦合" << std::endl;
    std::cout << "   - 设备类与能力类紧密绑定" << std::endl;
    std::cout << "   - 修改能力类需要修改所有继承的设备类" << std::endl;
    
    std::cout << "\n4. 违反单一职责原则" << std::endl;
    std::cout << "   - 设备类承担了多个职责" << std::endl;
    std::cout << "   - 需要知道所有父类的接口" << std::endl;
    
    std::cout << "\n5. 多继承问题" << std::endl;
    std::cout << "   - 可能导致菱形继承问题" << std::endl;
    std::cout << "   - 继承层次复杂，难以维护" << std::endl;
    
    std::cout << "\n6. 难以扩展" << std::endl;
    std::cout << "   - 添加新能力组合必须创建新类" << std::endl;
    std::cout << "   - 无法灵活配置设备能力" << std::endl;
    
    std::cout << "\n【具体问题】" << std::endl;
    std::cout << "问题场景：" << std::endl;
    std::cout << "  - 如果需要创建一个只有网络能力的设备？" << std::endl;
    std::cout << "    需要创建新的NetworkOnlyDevice类" << std::endl;
    std::cout << "  - 如果需要创建一个有网络+传感器+显示的设备？" << std::endl;
    std::cout << "    需要创建新的NetworkSensorDisplayDevice类" << std::endl;
    std::cout << "  - 如果需要在运行时动态添加能力？" << std::endl;
    std::cout << "    继承无法实现" << std::endl;
    
    std::cout << "\n【后果】" << std::endl;
    std::cout << "✗ 类数量爆炸（每种能力组合都需要一个类）" << std::endl;
    std::cout << "✗ 灵活性差（运行时无法改变）" << std::endl;
    std::cout << "✗ 耦合度高（紧密绑定）" << std::endl;
    std::cout << "✗ 难以维护（继承层次复杂）" << std::endl;
    std::cout << "✗ 难以扩展（需要创建新类）" << std::endl;
    std::cout << "✗ 违反SRP（多职责）" << std::endl;
}

void demonstrateGoodDesign() {
    printSeparator("好设计演示 - 使用组合复用（遵循组合复用原则）");
    
    std::cout << "\n【场景1：创建智能设备（使用组合）】" << std::endl;
    auto smartDevice = good_design::DeviceFactory::createSmartDevice("smart_001");
    smartDevice->performTask();
    
    std::cout << "\n【场景2：创建工业设备（使用组合）】" << std::endl;
    auto industrialDevice = good_design::DeviceFactory::createIndustrialDevice("industrial_001");
    industrialDevice->performTask();
    
    std::cout << "\n【场景3：创建IoT设备（使用组合）】" << std::endl;
    auto iotDevice = good_design::DeviceFactory::createIoTDevice("iot_001");
    iotDevice->performTask();
    
    std::cout << "\n【场景4：创建自定义设备（灵活组合）】" << std::endl;
    auto customDevice = good_design::DeviceFactory::createCustomDevice(
        "custom_001",
        "Bluetooth",
        "SD",
        "/mnt/sd/custom.txt",
        "Pressure"
    );
    customDevice->performTask();
    
    std::cout << "\n【场景5：设备管理器统一管理】" << std::endl;
    good_design::DeviceManager manager;
    manager.addDevice(std::move(smartDevice));
    manager.addDevice(std::move(industrialDevice));
    manager.addDevice(std::move(iotDevice));
    manager.addDevice(std::move(customDevice));
    manager.executeAllDevices();
    
    std::cout << "\n【场景6：运行时动态添加能力】" << std::endl;
    manager.addCapabilityToDevice(
        "iot_001",
        std::make_unique<good_design::NetworkCapability>("LoRaWAN")
    );
    
    auto iotDev = manager.findDevice("iot_001");
    if (iotDev) {
        std::cout << "IoT设备的能力：";
        for (const auto& type : iotDev->getCapabilityTypes()) {
            std::cout << " " << type;
        }
        std::cout << std::endl;
        iotDev->performTask();
    }
    
    std::cout << "\n【场景7：运行时动态移除能力】" << std::endl;
    manager.removeCapabilityFromDevice("custom_001", "storage");
    
    auto customDev = manager.findDevice("custom_001");
    if (customDev) {
        std::cout << "Custom设备的能力：";
        for (const auto& type : customDev->getCapabilityTypes()) {
            std::cout << " " << type;
        }
        std::cout << std::endl;
    }
    
    std::cout << "\n【优势分析】" << std::endl;
    std::cout << "使用组合复用的优势：" << std::endl;
    std::cout << "1. 类数量少" << std::endl;
    std::cout << "   - 只需要1个Device基类" << std::endl;
    std::cout << "   - 只需要3个Capability实现类" << std::endl;
    std::cout << "   - 不需要为每种组合创建新类" << std::endl;
    
    std::cout << "\n2. 组合在运行时确定" << std::endl;
    std::cout << "   - 运行时可以动态添加能力" << std::endl;
    std::cout << "   - 运行时可以动态移除能力" << std::endl;
    std::cout << "   - 灵活配置设备能力" << std::endl;
    
    std::cout << "\n3. 松耦合" << std::endl;
    std::cout << "   - 设备类与能力类通过接口解耦" << std::endl;
    std::cout << "   - 修改能力类不影响设备类" << std::endl;
    
    std::cout << "\n4. 符合单一职责原则" << std::endl;
    std::cout << "   - Device类只负责管理能力" << std::endl;
    std::cout << "   - Capability类只负责具体功能" << std::endl;
    
    std::cout << "\n5. 避免多继承问题" << std::endl;
    std::cout << "   - 没有多重继承" << std::endl;
    std::cout << "   - 类层次简单，易于维护" << std::endl;
    
    std::cout << "\n6. 易于扩展" << std::endl;
    std::cout << "   - 添加新能力只需实现ICapability" << std::endl;
    std::cout << "   - 不需要修改现有类" << std::endl;
    
    std::cout << "\n【具体优势】" << std::endl;
    std::cout << "优势场景：" << std::endl;
    std::cout << "  - 如果需要创建一个只有网络能力的设备？" << std::endl;
    std::cout << "    只需：new Device() + addCapability(new NetworkCapability())" << std::endl;
    std::cout << "  - 如果需要创建一个有网络+传感器+显示的设备？" << std::endl;
    std::cout << "    只需：new Device() + addCapabilities()" << std::endl;
    std::cout << "  - 如果需要在运行时动态添加能力？" << std::endl;
    std::cout << "    只需：device->addCapability()" << std::endl;
    
    std::cout << "\n【优势总结】" << std::endl;
    std::cout << "✓ 类数量少（不需要为每种组合创建类）" << std::endl;
    std::cout << "✓ 灵活性高（运行时可以改变）" << std::endl;
    std::cout << "✓ 耦合度低（通过接口解耦）" << std::endl;
    std::cout << "✓ 易于维护（类层次简单）" << std::endl;
    std::cout << "✓ 易于扩展（添加新能力不影响现有代码）" << std::endl;
    std::cout << "✓ 符合SRP（职责清晰）" << std::endl;
    std::cout << "✓ 符合OCP（对扩展开放）" << std::endl;
}

void demonstratePrinciple() {
    printSeparator("组合复用原则核心思想");
    
    std::cout << "\n【组合复用原则定义】" << std::endl;
    std::cout << "组合复用原则（Composite Reuse Principle）：" << std::endl;
    std::cout << "在软件设计中，应该优先使用组合（Composition）而不是继承" << std::endl;
    std::cout << "来达到代码复用的目的。" << std::endl;
    
    std::cout << "\n【核心思想】" << std::endl;
    std::cout << "组合优于继承（Favor Composition over Inheritance）" << std::endl;
    std::cout << "1. 组合：has-a关系（包含关系）" << std::endl;
    std::cout << "2. 继承：is-a关系（派生关系）" << std::endl;
    std::cout << "3. 当不确定时，优先选择组合" << std::endl;
    
    std::cout << "\n【组合 vs 继承对比】" << std::endl;
    std::cout << "组合（Composition）：" << std::endl;
    std::cout << "  • 运行时动态组合" << std::endl;
    std::cout << "  • 松耦合" << std::endl;
    std::cout << "  • 灵活性高" << std::endl;
    std::cout << "  • 易于测试" << std::endl;
    std::cout << "  • 避免类爆炸" << std::endl;
    
    std::cout << "\n继承（Inheritance）：" << std::endl;
    std::cout << "  • 编译时静态组合" << std::endl;
    std::cout << "  • 紧耦合" << std::endl;
    std::cout << "  • 灵活性低" << std::endl;
    std::cout << "  • 难以测试" << std::endl;
    std::cout << "  • 可能导致类爆炸" << std::endl;
    
    std::cout << "\n【何时使用组合】" << std::endl;
    std::cout << "1. 需要运行时改变行为" << std::endl;
    std::cout << "2. 需要灵活配置能力" << std::endl;
    std::cout << "3. 需要动态添加/移除功能" << std::endl;
    std::cout << "4. 不确定是否适合继承" << std::endl;
    std::cout << "5. 避免类层次爆炸" << std::endl;
    
    std::cout << "\n【何时使用继承】" << std::endl;
    std::cout << "1. 明确的is-a关系" << std::endl;
    std::cout << "2. 需要利用多态" << std::endl;
    std::cout << "3. 子类确实需要父类的所有功能" << std::endl;
    std::cout << "4. 层次结构清晰且稳定" << std::endl;
}

void demonstrateComparison() {
    printSeparator("设计对比分析");
    
    std::cout << "\n【类数量对比】" << std::endl;
    std::cout << "坏设计（使用继承）：" << std::endl;
    std::cout << "  - NetworkCapability" << std::endl;
    std::cout << "  - StorageCapability" << std::endl;
    std::cout << "  - SensorCapability" << std::endl;
    std::cout << "  - SmartDevice（继承3个Capability）" << std::endl;
    std::cout << "  - IndustrialDevice（继承2个Capability）" << std::endl;
    std::cout << "  - IoTDevice（继承2个Capability）" << std::endl;
    std::cout << "  - NetworkStorageDevice（继承2个Capability）" << std::endl;
    std::cout << "  - ...（每种组合都需要新类）" << std::endl;
    std::cout << "  总计：7+ 个类" << std::endl;
    
    std::cout << "\n好设计（使用组合）：" << std::endl;
    std::cout << "  - ICapability（接口）" << std::endl;
    std::cout << "  - NetworkCapability（实现）" << std::endl;
    std::cout << "  - StorageCapability（实现）" << std::endl;
    std::cout << "  - SensorCapability（实现）" << std::endl;
    std::cout << "  - Device（基类）" << std::endl;
    std::cout << "  - SmartDevice（继承Device）" << std::endl;
    std::cout << "  - IndustrialDevice（继承Device）" << std::endl;
    std::cout << "  - IoTDevice（继承Device）" << std::endl;
    std::cout << "  - DeviceFactory（工厂）" << std::endl;
    std::cout << "  - DeviceManager（管理器）" << std::endl;
    std::cout << "  总计：10 个类（固定）" << std::endl;
    
    std::cout << "\n【灵活性对比】" << std::endl;
    std::cout << "坏设计:" << std::endl;
    std::cout << "  - 编译时确定能力组合" << std::endl;
    std::cout << "  - 运行时无法改变" << std::endl;
    std::cout << "  - 添加新组合需要创建新类" << std::endl;
    
    std::cout << "\n好设计:" << std::endl;
    std::cout << "  - 运行时动态添加能力" << std::endl;
    std::cout << "  - 运行时动态移除能力" << std::endl;
    std::cout << "  - 灵活配置能力组合" << std::endl;
    
    std::cout << "\n【耦合度对比】" << std::endl;
    std::cout << "坏设计:" << std::endl;
    std::cout << "  - 设备类直接继承能力类" << std::endl;
    std::cout << "  - 紧密耦合" << std::endl;
    std::cout << "  - 修改能力类影响所有设备类" << std::endl;
    
    std::cout << "\n好设计:" << std::endl;
    std::cout << "  - 设备类通过接口依赖能力类" << std::endl;
    std::cout << "  - 松散耦合" << std::endl;
    std::cout << "  - 修改能力类不影响设备类" << std::endl;
}

int main() {
    std::cout << "=== 组合复用原则演示程序 ===" << std::endl;
    std::cout << "Composite Reuse Principle Demonstration" << std::endl;
    std::cout << "场景：嵌入式设备能力组装" << std::endl;
    
    try {
        // 演示坏设计
        demonstrateBadDesign();
        
        // 演示好设计
        demonstrateGoodDesign();
        
        // 演示组合复用原则核心思想
        demonstratePrinciple();
        
        // 对比分析
        demonstrateComparison();
        
        printSeparator("演示完成");
        std::cout << "\n【组合复用原则核心思想】" << std::endl;
        std::cout << "组合优于继承（Favor Composition over Inheritance）" << std::endl;
        
        std::cout << "\n【关键要点】" << std::endl;
        std::cout << "1. 优先使用组合而不是继承来复用代码" << std::endl;
        std::cout << "2. 组合在运行时确定，继承在编译时确定" << std::endl;
        std::cout << "3. 组合提供更高的灵活性和可扩展性" << std::endl;
        std::cout << "4. 通过接口实现松耦合" << std::endl;
        std::cout << "5. 避免类爆炸和继承层次复杂性" << std::endl;
        
        std::cout << "\n【何时使用组合】" << std::endl;
        std::cout << "• 需要运行时动态改变行为" << std::endl;
        std::cout << "• 需要灵活配置能力组合" << std::endl;
        std::cout << "• 不确定是否适合继承" << std::endl;
        
        std::cout << "\n【何时使用继承】" << std::endl;
        std::cout << "• 明确的is-a关系" << std::endl;
        std::cout << "• 需要利用多态" << std::endl;
        std::cout << "• 子类确实需要父类的所有功能" << std::endl;
        
        std::cout << "\n【实际应用】" << std::endl;
        std::cout << "• 嵌入式系统设计" << std::endl;
        std::cout << "• 游戏开发（组件系统）" << std::endl;
        std::cout << "• 插件架构" << std::endl;
        std::cout << "• 微服务架构" << std::endl;
        std::cout << "• UI框架设计" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "程序执行出错: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
