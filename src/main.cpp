#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <memory>
#include <chrono>

#include "sys_sentinel/queue.h"
#include "sys_sentinel/logger.h"         
#include "sys_sentinel/console_logger.h"
#include "sys_sentinel/file_logger.h"

int main() {
    std::cout << "SysSentinel: Starting Multi-Threaded Monitoring..." << std::endl;

    // 1. Setup the Logger Manager (The Central Hub)
    LoggerManager loggerManager;

    // 2. Create and Register Observers
    // We use shared_ptr because LoggerManager needs to hold them
    auto consoleLogger = std::make_shared<ConsoleLogger>();
    auto fileLogger = std::make_shared<FileLogger>("sys_sentinel.log");

    loggerManager.addLogger(consoleLogger);
    loggerManager.addLogger(fileLogger);

    // 3. Setup the Thread-Safe Queue
    ThreadSafeQueue<std::string> logQueue;

    // 4. Start Producer Thread (Simulating System Events)
    std::jthread producer([&logQueue]() {
        const std::vector<std::string> events = {
            "System boot sequence initiated", 
            "Loading kernel modules...",
            "WARNING: Disk space below 10%", 
            "Network interface eth0 up", 
            "ERROR: Connection to database timed out",
            "Retrying connection...",
            "Connection established",
            "User 'admin' logged in"
        };

        for (const auto& event : events) {
            logQueue.push(event);
            // Simulate variable work time
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        logQueue.push("EXIT"); 
    });

    // 5. Start Consumer Thread (The Processor)
    std::jthread consumer([&logQueue, &loggerManager]() {
        while (true) {
            std::string logMsg;
            logQueue.pop(logMsg); // Blocks here until data arrives

            if (logMsg == "EXIT") break;

            // Simple "AI" - Detect severity based on string content
            LogLevel level = LogLevel::INFO;
            if (logMsg.find("ERROR") != std::string::npos) {
                level = LogLevel::ERROR;
            } else if (logMsg.find("WARNING") != std::string::npos) {
                level = LogLevel::WARNING;
            }

            // Dispatch to all registered loggers
            loggerManager.log(level, logMsg);
        }
    });

    // Main thread waits here until jthreads finish automatically
    std::cout << "Monitoring active. Press Ctrl+C to force quit or wait for completion." << std::endl;
    return 0;
}