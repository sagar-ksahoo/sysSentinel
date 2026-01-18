#include "sys_sentinel/monitor.h"

#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/processor_info.h>
#include <sys/statvfs.h>  // Required for disk stats
#include <sys/sysctl.h>
#include <sys/types.h>

#include <iostream>
#include <vector>

SystemMonitor::SystemMonitor() {
    // Initialize initial CPU state to avoid weird 0% or 100% spikes on first run
    calculateCpuLoad(previousIdleTicks, previousTotalTicks);
}

void SystemMonitor::calculateCpuLoad(unsigned long long& idleTicks,
                                     unsigned long long& totalTicks) {
    host_cpu_load_info_data_t cpuinfo;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;

    // Call the OS Kernel to get stats
    if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuinfo, &count) ==
        KERN_SUCCESS) {
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
    SystemMetrics metrics{}; 

    // --- 1. Get CPU Usage ---
    unsigned long long currentTotalTicks = 0;
    unsigned long long currentIdleTicks = 0;

    calculateCpuLoad(currentIdleTicks, currentTotalTicks);

    unsigned long long totalDiff = currentTotalTicks - previousTotalTicks;
    unsigned long long idleDiff = currentIdleTicks - previousIdleTicks;

    if (totalDiff > 0) {
        metrics.cpuLoad = 100.0 * (1.0 - ((double)idleDiff / totalDiff));
    }

    previousTotalTicks = currentTotalTicks;
    previousIdleTicks = currentIdleTicks;

    // --- 2. Get GLOBAL System Memory Usage (NEW) ---
    // We need the page size (usually 16KB on Apple Silicon) to convert "pages" to "bytes"
    vm_size_t pageSize;
    mach_port_t machPort = mach_host_self();
    if (host_page_size(machPort, &pageSize) == KERN_SUCCESS) {
        vm_statistics64_data_t vmStats;
        mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;

        if (host_statistics64(machPort, HOST_VM_INFO64, (host_info64_t)&vmStats, &count) ==
            KERN_SUCCESS) {
            // "Active" + "Wired" is a good approximation of "Used" memory on macOS
            // (macOS memory accounting is complex, but this is standard for monitors)
            long long usedPages = vmStats.active_count + vmStats.wire_count;
            metrics.memoryUsed = usedPages * pageSize;
        }
    }

    // Getting Total System RAM (Already correct)
    uint64_t memsize = 0;
    size_t size = sizeof(memsize);
    if (sysctlbyname("hw.memsize", &memsize, &size, NULL, 0) == 0) {
        metrics.memoryTotal = memsize;
    }

    // --- 3. Get Disk Usage ---
    struct statvfs diskData;
    if (statvfs("/", &diskData) == 0) {
        metrics.diskTotal = (size_t)diskData.f_blocks * diskData.f_frsize;
        metrics.diskFree = (size_t)diskData.f_bavail * diskData.f_frsize;
    }

    return metrics;
}