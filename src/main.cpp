#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <memory>
#include <chrono>

// Include our modular headers
#include "sys_sentinel/queue.h"
#include "sys_sentinel/logger.h"
#include "sys_sentinel/file_logger.h"
#include "sys_sentinel/dashboard.h"

int main() {
    // 1. Setup Dashboard (Handles Ncurses UI)
    // As soon as this is created, your terminal will clear and enter "Graphical" mode
    auto dashboard = std::make_shared<Dashboard>();

    // 2. Setup Logger Manager
    LoggerManager loggerManager;
    
    // The Dashboard is now a logger! It will show logs on the screen.
    loggerManager.addLogger(dashboard); 
    
    // We also keep the FileLogger so you have a permanent record on disk
    loggerManager.addLogger(std::make_shared<FileLogger>("sys_sentinel.log"));

    // 3. Setup the Thread-Safe Queue
    ThreadSafeQueue<std::string> logQueue;

    // 4. Producer Thread (Real System Monitor)
    // We pass 'dashboard' here so we can update the CPU/RAM bars directly
    std::jthread producer([&logQueue, dashboard]() {
        SystemMonitor monitor;
        
        // Let the user know monitoring has started
        logQueue.push("SysSentinel Service Started...");

        while (true) {
            // Get Real Data (CPU, RAM, Disk)
            SystemMetrics metrics = monitor.getMetrics();
            
            // Update the Visual Bars on the Dashboard
            dashboard->updateStats(metrics);

            // --- Logic to auto-generate Alerts ---
            
            // Example: High CPU Alert
            if (metrics.cpuLoad > 50.0) {
                 logQueue.push("WARNING: High CPU Load Detected!");
            }
            
            // Example: Low Disk Alert (Less than 10GB free)
            // 10GB = 10 * 1024 * 1024 * 1024 bytes
            if (metrics.diskFree < (10ULL * 1024 * 1024 * 1024)) { 
                logQueue.push("WARNING: Low Disk Space (< 10GB)");
            }

            // Sleep for 500ms (Update UI twice a second)
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });

    // 5. Consumer Thread (Processes Log Strings)
    std::jthread consumer([&logQueue, &loggerManager]() {
        while (true) {
            std::string logMsg;
            logQueue.pop(logMsg); // Blocks until a log arrives

            // Basic Severity Detection
            LogLevel level = LogLevel::INFO;
            if (logMsg.find("WARNING") != std::string::npos) level = LogLevel::WARNING;
            if (logMsg.find("ERROR") != std::string::npos) level = LogLevel::ERROR;

            // Send to both Dashboard (Screen) and File (Disk)
            loggerManager.log(level, logMsg);
        }
    });

    // 6. Keep Main Thread Alive
    // Since we are in Ncurses mode, we loop forever until the user kills the app.
    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}