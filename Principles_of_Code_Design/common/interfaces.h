#pragma once

#include <string>
#include <vector>
#include <memory>

namespace design_principles {

// 日志级别枚举
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

// 数据验证接口
class IDataValidator {
public:
    virtual ~IDataValidator() = default;
    virtual bool validate(const std::string& data) = 0;
    virtual std::vector<std::string> getErrors() const = 0;
    virtual void clearErrors() = 0;
};

// 日志记录接口
class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(LogLevel level, const std::string& message) = 0;
    virtual void setLevel(LogLevel level) = 0;
    virtual LogLevel getLevel() const = 0;
};

// 报告生成接口
class IReportGenerator {
public:
    virtual ~IReportGenerator() = default;
    virtual std::string generateReport(const std::vector<std::string>& data) = 0;
    virtual bool saveReport(const std::string& reportContent, const std::string& filename) = 0;
};

// 数据持久化接口
class IDataPersistence {
public:
    virtual ~IDataPersistence() = default;
    virtual bool saveData(const std::string& filename, const std::string& data) = 0;
    virtual bool loadData(const std::string& filename, std::string& data) = 0;
    virtual bool saveOrders(const std::string& filename, const std::vector<std::string>& orders) = 0;
    virtual bool loadOrders(const std::string& filename, std::vector<std::string>& orders) = 0;
};

} // namespace design_principles
