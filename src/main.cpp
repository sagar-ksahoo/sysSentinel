#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "sys_sentinel/console_logger.h"
#include "sys_sentinel/file_logger.h"
#include "sys_sentinel/logger.h"
#include "sys_sentinel/queue.h"
#include "sys_sentinel/monitor.h"

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

    // 4. Start Producer Thread (Real System Monitor)
    std::jthread producer([&logQueue]() {
        SystemMonitor monitor;

        while (true) {
            SystemMetrics metrics = monitor.getMetrics();

            // Create a formatted log string using C++20 std::format (or string concatenation)
            // Note: If std::format isn't available yet on your Clang, use basic string building
            std::string logMsg = "[SYSTEM] CPU: " + std::to_string(metrics.cpuLoad) + "% | " +
                                 "RAM Usage: " + std::to_string(metrics.memoryUsed / 1024 / 1024) +
                                 " MB";

            // Simple Logic: Alert if CPU spikes (Simulated threshold for demo)
            if (metrics.cpuLoad > 10.0) {  // Low threshold just to see it trigger
                logMsg += " [WARNING: High CPU Load]";
            }

            logQueue.push(logMsg);

            // Poll every 1 second
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    // 5. Start Consumer Thread (The Processor)
    std::jthread consumer([&logQueue, &loggerManager]() {
        while (true) {
            std::string logMsg;
            logQueue.pop(logMsg);  // Blocks here until data arrives

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
    std::cout << "Monitoring active. Press Ctrl+C to force quit or wait for completion."
              << std::endl;
    return 0;
}