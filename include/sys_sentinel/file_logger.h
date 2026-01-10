#ifndef SYS_SENTINEL_FILE_LOGGER_H
#define SYS_SENTINEL_FILE_LOGGER_H

#include "logger.h"
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip> // Required for std::put_time

class FileLogger : public ILogger {
public:
    explicit FileLogger(const std::string& filename) {
        // Open in Append mode so we don't overwrite previous logs on restart
        m_file.open(filename, std::ios::out | std::ios::app);
        if (!m_file.is_open()) {
            throw std::runtime_error("SysSentinel Error: Unable to open log file: " + filename);
        }
    }

    ~FileLogger() {
        if (m_file.is_open()) {
            m_file.close();
        }
    }

    void log(LogLevel level, const std::string& message) override {
        std::lock_guard<std::mutex> lock(m_fileMutex);
        if (m_file.is_open()) {
            // Get current time
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            
            // Format: [YYYY-MM-DD HH:MM:SS] [LEVEL]: Message
            m_file << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] "
                   << "[" << levelToString(level) << "]: " 
                   << message << std::endl;
        }
    }

private:
    // Helper to convert Enum to String for file output
    std::string levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::INFO:    return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR:   return "ERROR";
            case LogLevel::DEBUG:   return "DEBUG";
            default:                return "UNKNOWN";
        }
    }

    std::ofstream m_file;
    std::mutex m_fileMutex;
};

#endif