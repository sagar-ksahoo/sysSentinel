#ifndef SYS_SENTINEL_CONSOLE_LOGGER_H
#define SYS_SENTINEL_CONSOLE_LOGGER_H

#include <iostream>
#include <mutex>

#include "logger.h"

class ConsoleLogger : public ILogger {
   public:
    void log(LogLevel level, const std::string& message) override {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::string prefix;
        std::string colorCode;

        switch (level) {
            case LogLevel::INFO:
                prefix = "[INFO] ";
                colorCode = "\033[34m";  // Blue
                break;
            case LogLevel::WARNING:
                prefix = "[WARN] ";
                colorCode = "\033[33m";  // Yellow
                break;
            case LogLevel::ERROR:
                prefix = "[ERROR] ";
                colorCode = "\033[31m";  // Red
                break;
            case LogLevel::DEBUG:
                prefix = "[DEBUG] ";
                colorCode = "\033[36m";  // Cyan
                break;
        }
        std::cout << colorCode << prefix << message << "\033[0m" << std::endl;  // Reset color
    }

   private:
    static std::mutex m_mutex;
};

std::mutex ConsoleLogger::m_mutex;

#endif