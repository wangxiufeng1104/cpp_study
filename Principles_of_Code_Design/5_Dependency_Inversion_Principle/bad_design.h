#pragma once

#include <string>
#include <iostream>
#include <stdexcept>

namespace bad_design {

/**
 * 坏设计示例：违反依赖倒置原则
 * 
 * 问题：高层模块直接依赖低层模块，而不是依赖抽象
 * 导致：难以替换低层模块、难以测试、违反开闭原则
 */

// 低层模块：文件存储
class FileStorage {
private:
    std::string filename_;
    
public:
    FileStorage(const std::string& filename) 
        : filename_(filename) {
        std::cout << "[Bad Design] FileStorage created: " << filename << std::endl;
    }
    
    bool save(const std::string& data) {
        std::cout << "[Bad Design] FileStorage saving to: " << filename_ << std::endl;
        std::cout << "[Bad Design] Data: " << data << std::endl;
        return true;
    }
    
    bool load(std::string& data) {
        std::cout << "[Bad Design] FileStorage loading from: " << filename_ << std::endl;
        data = "{\"loaded\":true}";
        return true;
    }
};

// 低层模块：数据库存储
class DatabaseStorage {
private:
    std::string connectionString_;
    
public:
    DatabaseStorage(const std::string& connStr)
        : connectionString_(connStr) {
        std::cout << "[Bad Design] DatabaseStorage created: " << connStr << std::endl;
    }
    
    bool save(const std::string& data) {
        std::cout << "[Bad Design] DatabaseStorage saving to: " << connectionString_ << std::endl;
        std::cout << "[Bad Design] Data: " << data << std::endl;
        return true;
    }
    
    bool load(std::string& data) {
        std::cout << "[Bad Design] DatabaseStorage loading from: " << connectionString_ << std::endl;
        data = "{\"loaded\":true}";
        return true;
    }
};

// 低层模块：云存储
class CloudStorage {
private:
    std::string apiUrl_;
    std::string apiKey_;
    
public:
    CloudStorage(const std::string& apiUrl, const std::string& apiKey)
        : apiUrl_(apiUrl), apiKey_(apiKey) {
        std::cout << "[Bad Design] CloudStorage created: " << apiUrl << std::endl;
    }
    
    bool save(const std::string& data) {
        std::cout << "[Bad Design] CloudStorage saving to: " << apiUrl_ << std::endl;
        std::cout << "[Bad Design] Data: " << data << std::endl;
        return true;
    }
    
    bool load(std::string& data) {
        std::cout << "[Bad Design] CloudStorage loading from: " << apiUrl_ << std::endl;
        data = "{\"loaded\":true}";
        return true;
    }
};

// 高层模块：设备管理器
// 问题：直接依赖低层模块（FileStorage、DatabaseStorage等）
class DeviceManager {
private:
    FileStorage storage_;  // 直接依赖具体的FileStorage
    std::string deviceId_;
    
public:
    DeviceManager(const std::string& deviceId)
        : storage_("device_data.txt"), deviceId_(deviceId) {
        std::cout << "[Bad Design] DeviceManager created: " << deviceId << std::endl;
    }
    
    void saveData(const std::string& data) {
        std::cout << "[Bad Design] DeviceManager saving data..." << std::endl;
        storage_.save(data);
    }
    
    void loadData(std::string& data) {
        std::cout << "[Bad Design] DeviceManager loading data..." << std::endl;
        storage_.load(data);
    }
    
    std::string getDeviceId() const {
        return deviceId_;
    }
};

// 高层模块：数据处理器
// 问题：直接依赖DatabaseStorage
class DataProcessor {
private:
    DatabaseStorage storage_;  // 直接依赖具体的DatabaseStorage
    
public:
    DataProcessor(const std::string& connStr)
        : storage_(connStr) {
        std::cout << "[Bad Design] DataProcessor created" << std::endl;
    }
    
    void processData(const std::string& data) {
        std::cout << "[Bad Design] DataProcessor processing data..." << std::endl;
        storage_.save(data);
    }
};

// 高层模块：云同步器
// 问题：直接依赖CloudStorage
class CloudSyncer {
private:
    CloudStorage storage_;  // 直接依赖具体的CloudStorage
    
public:
    CloudSyncer(const std::string& apiUrl, const std::string& apiKey)
        : storage_(apiUrl, apiKey) {
        std::cout << "[Bad Design] CloudSyncer created" << std::endl;
    }
    
    void syncData(const std::string& data) {
        std::cout << "[Bad Design] CloudSyncer syncing data..." << std::endl;
        storage_.save(data);
    }
};

} // namespace bad_design
