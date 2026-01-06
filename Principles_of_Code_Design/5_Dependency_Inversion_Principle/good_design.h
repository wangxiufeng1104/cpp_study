#pragma once

#include <string>
#include <iostream>
#include <memory>
#include <vector>

namespace good_design {

/**
 * 好设计示例：遵循依赖倒置原则
 * 
 * 优势：高层模块和低层模块都依赖抽象
 * 导致：易于替换实现、易于测试、符合开闭原则
 */

// 抽象：存储接口
class IStorage {
public:
    virtual ~IStorage() = default;
    virtual bool save(const std::string& data) = 0;
    virtual bool load(std::string& data) = 0;
};

// 低层模块：文件存储（依赖抽象）
class FileStorage : public IStorage {
private:
    std::string filename_;
    
public:
    FileStorage(const std::string& filename) 
        : filename_(filename) {
        std::cout << "[Good Design] FileStorage created: " << filename << std::endl;
    }
    
    bool save(const std::string& data) override {
        std::cout << "[Good Design] FileStorage saving to: " << filename_ << std::endl;
        std::cout << "[Good Design] Data: " << data << std::endl;
        return true;
    }
    
    bool load(std::string& data) override {
        std::cout << "[Good Design] FileStorage loading from: " << filename_ << std::endl;
        data = "{\"loaded\":true,\"source\":\"file\"}";
        return true;
    }
};

// 低层模块：数据库存储（依赖抽象）
class DatabaseStorage : public IStorage {
private:
    std::string connectionString_;
    
public:
    DatabaseStorage(const std::string& connStr)
        : connectionString_(connStr) {
        std::cout << "[Good Design] DatabaseStorage created: " << connStr << std::endl;
    }
    
    bool save(const std::string& data) override {
        std::cout << "[Good Design] DatabaseStorage saving to: " << connectionString_ << std::endl;
        std::cout << "[Good Design] Data: " << data << std::endl;
        return true;
    }
    
    bool load(std::string& data) override {
        std::cout << "[Good Design] DatabaseStorage loading from: " << connectionString_ << std::endl;
        data = "{\"loaded\":true,\"source\":\"database\"}";
        return true;
    }
};

// 低层模块：云存储（依赖抽象）
class CloudStorage : public IStorage {
private:
    std::string apiUrl_;
    std::string apiKey_;
    
public:
    CloudStorage(const std::string& apiUrl, const std::string& apiKey)
        : apiUrl_(apiUrl), apiKey_(apiKey) {
        std::cout << "[Good Design] CloudStorage created: " << apiUrl << std::endl;
    }
    
    bool save(const std::string& data) override {
        std::cout << "[Good Design] CloudStorage saving to: " << apiUrl_ << std::endl;
        std::cout << "[Good Design] Data: " << data << std::endl;
        return true;
    }
    
    bool load(std::string& data) override {
        std::cout << "[Good Design] CloudStorage loading from: " << apiUrl_ << std::endl;
        data = "{\"loaded\":true,\"source\":\"cloud\"}";
        return true;
    }
};

// 低层模块：内存存储（依赖抽象）
class MemoryStorage : public IStorage {
private:
    std::string data_;
    
public:
    MemoryStorage() {
        std::cout << "[Good Design] MemoryStorage created" << std::endl;
    }
    
    bool save(const std::string& data) override {
        std::cout << "[Good Design] MemoryStorage saving to memory" << std::endl;
        std::cout << "[Good Design] Data: " << data << std::endl;
        data_ = data;
        return true;
    }
    
    bool load(std::string& data) override {
        std::cout << "[Good Design] MemoryStorage loading from memory" << std::endl;
        data = data_;
        return !data_.empty();
    }
};

// 高层模块：设备管理器（依赖抽象）
class DeviceManager {
private:
    IStorage& storage_;  // 依赖抽象IStorage，而非具体实现
    std::string deviceId_;
    
public:
    DeviceManager(IStorage& storage, const std::string& deviceId)
        : storage_(storage), deviceId_(deviceId) {
        std::cout << "[Good Design] DeviceManager created: " << deviceId << std::endl;
    }
    
    void saveData(const std::string& data) {
        std::cout << "[Good Design] DeviceManager saving data..." << std::endl;
        storage_.save(data);
    }
    
    void loadData(std::string& data) {
        std::cout << "[Good Design] DeviceManager loading data..." << std::endl;
        storage_.load(data);
    }
    
    std::string getDeviceId() const {
        return deviceId_;
    }
};

// 高层模块：数据处理器（依赖抽象）
class DataProcessor {
private:
    IStorage& storage_;  // 依赖抽象IStorage，而非具体实现
    
public:
    DataProcessor(IStorage& storage)
        : storage_(storage) {
        std::cout << "[Good Design] DataProcessor created" << std::endl;
    }
    
    void processData(const std::string& data) {
        std::cout << "[Good Design] DataProcessor processing data..." << std::endl;
        std::cout << "[Good Design] Processing: " << data << std::endl;
        storage_.save(data);
    }
};

// 高层模块：云同步器（依赖抽象）
class CloudSyncer {
private:
    IStorage& storage_;  // 依赖抽象IStorage，而非具体实现
    
public:
    CloudSyncer(IStorage& storage)
        : storage_(storage) {
        std::cout << "[Good Design] CloudSyncer created" << std::endl;
    }
    
    void syncData(const std::string& data) {
        std::cout << "[Good Design] CloudSyncer syncing data..." << std::endl;
        std::cout << "[Good Design] Syncing: " << data << std::endl;
        storage_.save(data);
    }
};

// 依赖注入工厂：创建存储实例
class StorageFactory {
public:
    static std::unique_ptr<IStorage> createFileStorage(const std::string& filename) {
        return std::make_unique<FileStorage>(filename);
    }
    
    static std::unique_ptr<IStorage> createDatabaseStorage(const std::string& connStr) {
        return std::make_unique<DatabaseStorage>(connStr);
    }
    
    static std::unique_ptr<IStorage> createCloudStorage(
        const std::string& apiUrl, 
        const std::string& apiKey) {
        return std::make_unique<CloudStorage>(apiUrl, apiKey);
    }
    
    static std::unique_ptr<IStorage> createMemoryStorage() {
        return std::make_unique<MemoryStorage>();
    }
};

// Mock存储：用于单元测试
class MockStorage : public IStorage {
private:
    std::vector<std::string> savedData_;
    
public:
    MockStorage() {
        std::cout << "[Good Design] MockStorage created (for testing)" << std::endl;
    }
    
    ~MockStorage() override = default;
    
    bool save(const std::string& data) override {
        savedData_.push_back(data);
        std::cout << "[Good Design] MockStorage saved data #" << savedData_.size() << std::endl;
        return true;
    }
    
    bool load(std::string& data) override {
        if (!savedData_.empty()) {
            data = savedData_.back();
            return true;
        }
        return false;
    }
    
    size_t getSavedCount() const {
        return savedData_.size();
    }
    
    void clear() {
        savedData_.clear();
    }
};

} // namespace good_design
