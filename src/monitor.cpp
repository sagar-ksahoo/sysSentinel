#include "sys_sentinel/monitor.h"
#include <mach/mach.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/processor_info.h>
#include <mach/mach_host.h>
#include <iostream>
#include <vector>

SystemMonitor::SystemMonitor() {
    // Initialize initial CPU state to avoid weird 0% or 100% spikes on first run
    calculateCpuLoad(previousIdleTicks, previousTotalTicks);
}

void SystemMonitor::calculateCpuLoad(unsigned long long& idleTicks, unsigned long long& totalTicks) {
    host_cpu_load_info_data_t cpuinfo;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    
    // Call the OS Kernel to get stats
    if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuinfo, &count) == KERN_SUCCESS) {
        unsigned long long user = cpuinfo.cpu_ticks[CPU_STATE_USER];
        unsigned long long system = cpuinfo.cpu_ticks[CPU_STATE_SYSTEM];
        unsigned long long idle = cpuinfo.cpu_ticks[CPU_STATE_IDLE];
        unsigned long long nice = cpuinfo.cpu_ticks[CPU_STATE_NICE];

        idleTicks = idle;
        totalTicks = user + system + idle + nice;
    } else {
        totalTicks = 0;
        idleTicks = 0;
    }
}

SystemMetrics SystemMonitor::getMetrics() {
    SystemMetrics metrics{0.0, 0, 0};

    // --- 1. Get CPU Usage ---
    unsigned long long currentTotalTicks = 0;
    unsigned long long currentIdleTicks = 0;
    
    calculateCpuLoad(currentIdleTicks, currentTotalTicks);

    unsigned long long totalDiff = currentTotalTicks - previousTotalTicks;
    unsigned long long idleDiff = currentIdleTicks - previousIdleTicks;

    if (totalDiff > 0) {
        // Usage = (Total - Idle) / Total
        metrics.cpuLoad = 100.0 * (1.0 - ((double)idleDiff / totalDiff));
    }

    // Update state for next calculation
    previousTotalTicks = currentTotalTicks;
    previousIdleTicks = currentIdleTicks;

    // --- 2. Get Memory Usage ---
    mach_task_basic_info info;
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;

    // mach_task_self() gets stats for THIS process. 
    // To get Global system RAM is harder, so we start by monitoring OUR Own usage + Total System RAM
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &count) == KERN_SUCCESS) {
        metrics.memoryUsed = info.resident_size; // RAM actually used by this app
    }

    // Getting Total System RAM
    uint64_t memsize = 0;
    size_t size = sizeof(memsize);
    if (sysctlbyname("hw.memsize", &memsize, &size, NULL, 0) == 0) {
        metrics.memoryTotal = memsize;
    }

    return metrics;
}