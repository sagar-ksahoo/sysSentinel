#ifndef SYS_SENTINEL_LOGGER_H
#define SYS_SENTINEL_LOGGER_H

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <algorithm>

// 1. Enum for Log Severity (Crucial for filtering)
enum class LogLevel {
    INFO,
    WARNING,
    ERROR,
    DEBUG
};

// 2. The Abstract Interface (formerly IObserver)
class ILogger {
public:
    virtual ~ILogger() = default;
    
    // Pure virtual function: All loggers MUST implement this
    virtual void log(LogLevel level, const std::string& message) = 0;
};

// 3. The Subject (Manager) that holds the list of loggers
class LoggerManager {
public:
    void addLogger(std::shared_ptr<ILogger> logger) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_loggers.push_back(logger);
    }

    // Notify all registered loggers
    void log(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& logger : m_loggers) {
            logger->log(level, message);
        } 
    }

private:
    std::mutex m_mutex;
    std::vector<std::shared_ptr<ILogger>> m_loggers;
};

#endif