#ifndef SYS_SENTINEL_DASHBOARD_H
#define SYS_SENTINEL_DASHBOARD_H

#include "logger.h"
#include "monitor.h"
#include <ncurses.h>
#include <vector>
#include <mutex>
#include <deque>

class Dashboard : public ILogger {
public:
    Dashboard();
    ~Dashboard(); // Destructor handles cleanup (endwin)

    // Update the stats area
    void updateStats(const SystemMetrics& metrics);

    // Implement the ILogger interface to capture logs
    void log(LogLevel level, const std::string& message) override;

    // Redraw the entire screen
    void draw();

private:
    void drawBorders();
    
    std::mutex m_uiMutex;
    SystemMetrics m_lastMetrics;
    
    // We keep the last 20 logs to scroll them
    std::deque<std::pair<LogLevel, std::string>> m_logBuffer;
    const size_t MAX_LOG_LINES = 20;
};

#endif