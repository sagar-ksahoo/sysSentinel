#ifndef SYS_SENTINEL_MONITOR_H
#define SYS_SENTINEL_MONITOR_H

#include <string>

struct SystemMetrics {
    double cpuLoad;       // Percentage (0.0 to 100.0)
    size_t memoryUsed;    // In Bytes
    size_t memoryTotal;   // In Bytes
};

class SystemMonitor {
public:
    SystemMonitor();
    
    // The main method we will call in our loop
    SystemMetrics getMetrics();

private:
    // Helpers for calculating CPU delta (CPU usage is a rate of change!)
    unsigned long long previousTotalTicks = 0;
    unsigned long long previousIdleTicks = 0;

    void calculateCpuLoad(unsigned long long& idleTicks, unsigned long long& totalTicks);
};

#endif