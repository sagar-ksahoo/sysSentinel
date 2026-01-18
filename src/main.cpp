#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "sys_sentinel/console_logger.h"
#include "sys_sentinel/file_logger.h"
#include "sys_sentinel/logger.h"
#include "sys_sentinel/monitor.h"
#include "sys_sentinel/queue.h"

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

            // Convert raw bytes to GB for readability
            double ramUsedGB = metrics.memoryUsed / (1024.0 * 1024.0 * 1024.0);
            double diskFreeGB = metrics.diskFree / (1024.0 * 1024.0 * 1024.0);
            double diskTotalGB = metrics.diskTotal / (1024.0 * 1024.0 * 1024.0);
            double diskUsedPercent = 100.0 * (1.0 - (double)metrics.diskFree / metrics.diskTotal);

            // Format the log string
            // "CPU: 12.5% | RAM: 8.5 GB | Disk: 150/500 GB (30% Used)"
            std::string logMsg = "[SYSTEM] CPU: " + std::to_string(metrics.cpuLoad).substr(0, 4) +
                                 "% | " + "RAM: " + std::to_string(ramUsedGB).substr(0, 4) +
                                 " GB | " +
                                 "Disk Free: " + std::to_string(diskFreeGB).substr(0, 5) + " GB";

            // Add Alert Logic for Disk
            if (diskUsedPercent > 90.0) {
                logMsg += " [CRITICAL: Disk Space Low]";
            }

            logQueue.push(logMsg);
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