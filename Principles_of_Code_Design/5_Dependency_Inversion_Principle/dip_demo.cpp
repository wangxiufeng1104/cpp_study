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
    printSeparator("坏设计演示 - 违反依赖倒置原则");
    
    std::cout << "\n【场景1：使用文件存储】" << std::endl;
    {
        bad_design::DeviceManager manager("device_001");
        std::string data = "{\"temp\":25.5,\"humidity\":60.0}";
        manager.saveData(data);
        
        std::string loadedData;
        manager.loadData(loadedData);
    }
    
    std::cout << "\n【场景2：使用数据库存储】" << std::endl;
    {
        bad_design::DataProcessor processor("postgresql://localhost:5432/mydb");
        std::string data = "{\"status\":\"online\",\"version\":\"1.0\"}";
        processor.processData(data);
    }
    
    std::cout << "\n【场景3：使用云存储】" << std::endl;
    {
        bad_design::CloudSyncer syncer("https://api.example.com", "api_key_123");
        std::string data = "{\"action\":\"heartbeat\"}";
        syncer.syncData(data);
    }
    
    std::cout << "\n【问题分析】" << std::endl;
    std::cout << "违反依赖倒置原则的问题：" << std::endl;
    std::cout << "1. 高层模块直接依赖低层模块" << std::endl;
    std::cout << "   - DeviceManager直接依赖FileStorage" << std::endl;
    std::cout << "   - DataProcessor直接依赖DatabaseStorage" << std::endl;
    std::cout << "   - CloudSyncer直接依赖CloudStorage" << std::endl;
    
    std::cout << "\n2. 难以替换存储实现" << std::endl;
    std::cout << "   - 要换成DatabaseStorage，必须修改DeviceManager类" << std::endl;
    std::cout << "   - 要换成CloudStorage，必须修改DataProcessor类" << std::endl;
    std::cout << "   - 要换成FileStorage，必须修改CloudSyncer类" << std::endl;
    
    std::cout << "\n3. 违反开闭原则" << std::endl;
    std::cout << "   - 添加新存储方式需要修改高层模块" << std::endl;
    std::cout << "   - 修改高层模块可能引入新bug" << std::endl;
    std::cout << "   - 需要重新测试整个模块" << std::endl;
    
    std::cout << "\n4. 难以单元测试" << std::endl;
    std::cout << "   - 无法mock存储实现" << std::endl;
    std::cout << "   - 测试需要真实的文件或数据库" << std::endl;
    std::cout << "   - 测试速度慢、不稳定" << std::endl;
    
    std::cout << "\n【后果】" << std::endl;
    std::cout << "✗ 高层模块和低层模块紧密耦合" << std::endl;
    std::cout << "✗ 修改存储实现需要修改高层模块" << std::endl;
    std::cout << "✗ 违反开闭原则（对修改开放）" << std::endl;
    std::cout << "✗ 难以进行单元测试" << std::endl;
    std::cout << "✗ 代码复用性差" << std::endl;
    std::cout << "✗ 系统灵活性低" << std::endl;
}

void demonstrateGoodDesign() {
    printSeparator("好设计演示 - 遵循依赖倒置原则");
    
    std::cout << "\n【场景1：设备管理器使用文件存储】" << std::endl;
    {
        auto storage = good_design::StorageFactory::createFileStorage("device_data.txt");
        good_design::DeviceManager manager(*storage, "device_001");
        
        std::string data = "{\"temp\":25.5,\"humidity\":60.0}";
        manager.saveData(data);
        
        std::string loadedData;
        manager.loadData(loadedData);
    }
    
    std::cout << "\n【场景2：设备管理器使用数据库存储】" << std::endl;
    {
        auto storage = good_design::StorageFactory::createDatabaseStorage("postgresql://localhost:5432/mydb");
        good_design::DeviceManager manager(*storage, "device_001");
        
        std::string data = "{\"status\":\"online\",\"version\":\"1.0\"}";
        manager.saveData(data);
        
        std::string loadedData;
        manager.loadData(loadedData);
    }
    
    std::cout << "\n【场景3：数据处理器使用云存储】" << std::endl;
    {
        auto storage = good_design::StorageFactory::createCloudStorage(
            "https://api.example.com", "api_key_123");
        good_design::DataProcessor processor(*storage);
        
        std::string data = "{\"action\":\"heartbeat\"}";
        processor.processData(data);
    }
    
    std::cout << "\n【场景4：云同步器使用内存存储】" << std::endl;
    {
        auto storage = good_design::StorageFactory::createMemoryStorage();
        good_design::CloudSyncer syncer(*storage);
        
        std::string data = "{\"event\":\"temp_alert\"}";
        syncer.syncData(data);
    }
    
    std::cout << "\n【优势分析】" << std::endl;
    std::cout << "遵循依赖倒置原则的优势：" << std::endl;
    std::cout << "1. 高层模块和低层模块都依赖抽象" << std::endl;
    std::cout << "   - DeviceManager依赖IStorage抽象" << std::endl;
    std::cout << "   - DataProcessor依赖IStorage抽象" << std::endl;
    std::cout << "   - CloudSyncer依赖IStorage抽象" << std::endl;
    
    std::cout << "\n2. 易于替换存储实现" << std::endl;
    std::cout << "   - 切换存储实现只需修改依赖注入" << std::endl;
    std::cout << "   - 高层模块代码无需修改" << std::endl;
    std::cout << "   - 运行时可以动态切换" << std::endl;
    
    std::cout << "\n3. 符合开闭原则" << std::endl;
    std::cout << "   - 添加新存储方式无需修改高层模块" << std::endl;
    std::cout << "   - 只需创建新的存储实现类" << std::endl;
    std::cout << "   - 对扩展开放，对修改关闭" << std::endl;
    
    std::cout << "\n4. 易于单元测试" << std::endl;
    std::cout << "   - 可以使用MockStorage进行测试" << std::endl;
    std::cout << "   - 测试速度快、稳定" << std::endl;
    std::cout << "   - 隔离外部依赖" << std::endl;
    
    std::cout << "\n【优势总结】" << std::endl;
    std::cout << "✓ 高层模块和低层模块松耦合" << std::endl;
    std::cout << "✓ 易于替换低层模块实现" << std::endl;
    std::cout << "✓ 符合开闭原则（对扩展开放，对修改关闭）" << std::endl;
    std::cout << "✓ 易于进行单元测试" << std::endl;
    std::cout << "✓ 代码复用性高" << std::endl;
    std::cout << "✓ 系统灵活性高" << std::endl;
}

void demonstrateRuntimeSwitch() {
    printSeparator("运行时存储切换演示");
    
    std::cout << "\n【场景：根据网络条件动态切换存储】" << std::endl;
    
    // 创建设备管理器（高层模块）
    auto storage1 = good_design::StorageFactory::createFileStorage("device_data.txt");
    good_design::DeviceManager manager(*storage1, "device_001");
    
    std::cout << "\n阶段1：设备启动，使用文件存储" << std::endl;
    std::string data = "{\"temp\":25.5,\"status\":\"startup\"}";
    manager.saveData(data);
    
    std::cout << "\n阶段2：网络连接成功，切换到云存储" << std::endl;
    auto storage2 = good_design::StorageFactory::createCloudStorage(
        "https://api.example.com", "api_key_123");
    // 注意：这里模拟运行时切换（实际需要更复杂的实现）
    std::cout << "  ✓ 存储切换：FileStorage -> CloudStorage" << std::endl;
    
    std::cout << "\n阶段3：数据库连接建立，切换到数据库存储" << std::endl;
    auto storage3 = good_design::StorageFactory::createDatabaseStorage("postgresql://localhost:5432/mydb");
    std::cout << "  ✓ 存储切换：CloudStorage -> DatabaseStorage" << std::endl;
    
    std::cout << "\n【运行时切换的优势】" << std::endl;
    std::cout << "• 网络不稳定时使用文件存储" << std::endl;
    std::cout << "• 网络恢复时切换到云存储" << std::endl;
    std::cout << "• 需要高性能时切换到内存存储" << std::endl;
    std::cout << "• 需要持久化时切换到数据库存储" << std::endl;
    std::cout << "• 根据场景选择最优存储方式" << std::endl;
}

void demonstrateUnitTesting() {
    printSeparator("单元测试演示");
    
    std::cout << "\n【使用MockStorage进行单元测试】" << std::endl;
    
    // 创建Mock存储
    auto mockStorage = std::make_unique<good_design::MockStorage>();
    
    // 使用Mock存储创建设备管理器
    good_design::DeviceManager manager(*mockStorage, "device_001");
    
    std::cout << "\n测试场景1：保存3条数据" << std::endl;
    manager.saveData("{\"temp\":25.5,\"id\":1}");
    manager.saveData("{\"temp\":26.0,\"id\":2}");
    manager.saveData("{\"temp\":26.5,\"id\":3}");
    
    std::cout << "\n测试场景2：数据处理器处理数据" << std::endl;
    good_design::DataProcessor processor(*mockStorage);
    processor.processData("{\"status\":\"processed\"}");
    
    std::cout << "\n【单元测试的优势】" << std::endl;
    std::cout << "• 测试速度快（不需要真实IO）" << std::endl;
    std::cout << "• 测试稳定（不受外部因素影响）" << std::endl;
    std::cout << "• 可以验证数据是否保存" << std::endl;
    std::cout << "• 可以模拟各种场景" << std::endl;
    std::cout << "• 隔离外部依赖" << std::endl;
}

void demonstrateComparison() {
    printSeparator("设计对比分析");
    
    std::cout << "\n【依赖关系对比】" << std::endl;
    std::cout << "坏设计：" << std::endl;
    std::cout << "  高层模块（DeviceManager）" << std::endl;
    std::cout << "       ↓ 直接依赖" << std::endl;
    std::cout << "  低层模块（FileStorage）" << std::endl;
    std::cout << "  高层模块（DataProcessor）" << std::endl;
    std::cout << "       ↓ 直接依赖" << std::endl;
    std::cout << "  低层模块（DatabaseStorage）" << std::endl;
    
    std::cout << "\n好设计：" << std::endl;
    std::cout << "  高层模块（DeviceManager）" << std::endl;
    std::cout << "       ↓ 依赖抽象" << std::endl;
    std::cout << "  抽象（IStorage）" << std::endl;
    std::cout << "       ↑ 被低层模块实现" << std::endl;
    std::cout << "  低层模块（FileStorage、DatabaseStorage等）" << std::endl;
    
    std::cout << "\n【可维护性对比】" << std::endl;
    std::cout << "坏设计:" << std::endl;
    std::cout << "  - 修改存储实现需要修改高层模块" << std::endl;
    std::cout << "  - 高层模块和低层模块耦合紧密" << std::endl;
    std::cout << "  - 难以进行单元测试" << std::endl;
    std::cout << "  - 修改影响范围大" << std::endl;
    
    std::cout << "\n好设计:" << std::endl;
    std::cout << "  - 修改存储实现无需修改高层模块" << std::endl;
    std::cout << "  - 高层模块和低层模块松耦合" << std::endl;
    std::cout << "  - 易于进行单元测试（使用Mock）" << std::endl;
    std::cout << "  - 修改影响范围小" << std::endl;
    
    std::cout << "\n【扩展性对比】" << std::endl;
    std::cout << "坏设计:" << std::endl;
    std::cout << "  - 添加新存储方式需要修改高层模块" << std::endl;
    std::cout << "  - 违反开闭原则" << std::endl;
    std::cout << "  - 可能引入新bug" << std::endl;
    
    std::cout << "\n好设计:" << std::endl;
    std::cout << "  - 添加新存储方式只需创建新类" << std::endl;
    std::cout << "  - 符合开闭原则" << std::endl;
    std::cout << "  - 不会引入新bug到现有代码" << std::endl;
    
    std::cout << "\n【可测试性对比】" << std::endl;
    std::cout << "坏设计:" << std::endl;
    std::cout << "  - 无法mock存储实现" << std::endl;
    std::cout << "  - 测试需要真实IO操作" << std::endl;
    std::cout << "  - 测试速度慢、不稳定" << std::endl;
    
    std::cout << "\n好设计:" << std::endl;
    std::cout << "  - 可以使用MockStorage" << std::endl;
    std::cout << "  - 测试不需要真实IO" << std::endl;
    std::cout << "  - 测试速度快、稳定" << std::endl;
}

int main() {
    std::cout << "=== 依赖倒置原则演示程序 ===" << std::endl;
    std::cout << "Dependency Inversion Principle Demonstration" << std::endl;
    std::cout << "场景：嵌入式设备存储管理" << std::endl;
    
    try {
        // 演示坏设计
        demonstrateBadDesign();
        
        // 演示好设计
        demonstrateGoodDesign();
        
        // 演示运行时切换
        demonstrateRuntimeSwitch();
        
        // 演示单元测试
        demonstrateUnitTesting();
        
        // 对比分析
        demonstrateComparison();
        
        printSeparator("演示完成");
        std::cout << "\n【依赖倒置原则核心思想】" << std::endl;
        std::cout << "高层模块不应该依赖低层模块，两者都应该依赖抽象。" << std::endl;
        std::cout << "抽象不应该依赖细节，细节应该依赖抽象。" << std::endl;
        
        std::cout << "\n【关键要点】" << std::endl;
        std::cout << "1. 高层模块定义业务逻辑和接口" << std::endl;
        std::cout << "2. 低层模块实现高层定义的接口" << std::endl;
        std::cout << "3. 依赖倒置：高层依赖抽象，低层也依赖抽象" << std::endl;
        std::cout << "4. 依赖注入：通过构造函数注入抽象" << std::endl;
        std::cout << "5. 控制反转：由外部控制对象的创建" << std::endl;
        
        std::cout << "\n【实际应用】" << std::endl;
        std::cout << "• 依赖注入（构造函数、setter、接口注入）" << std::endl;
        std::cout << "• 依赖容器（Spring、Guice等）" << std::endl;
        std::cout << "• 插件架构" << std::endl;
        std::cout << "• 事件驱动架构" << std::endl;
        std::cout << "• 微服务架构" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "程序执行出错: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
