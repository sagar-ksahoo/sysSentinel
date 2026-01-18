#ifndef SYS_SENTINEL_MONITOR_H
#define SYS_SENTINEL_MONITOR_H

#include <string>

struct SystemMetrics {
    double cpuLoad = 0.0;    // Percentage (0.0 to 100.0)
    size_t memoryUsed = 0;   // In Bytes
    size_t memoryTotal = 0;  // In Bytes

    size_t diskTotal = 0;  // Bytes
    size_t diskFree = 0;   // Bytes
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