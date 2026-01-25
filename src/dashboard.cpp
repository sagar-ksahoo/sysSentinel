#include "sys_sentinel/dashboard.h"

#include <iomanip>
#include <sstream>

Dashboard::Dashboard() {
    initscr();      // Start Ncurses
    cbreak();       // Disable line buffering
    noecho();       // Don't show typing
    curs_set(0);    // Hide cursor
    start_color();  // Enable colors

    // Define Color Pairs
    init_pair(1, COLOR_CYAN, COLOR_BLACK);    // Borders
    init_pair(2, COLOR_GREEN, COLOR_BLACK);   // Good/Info
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);  // Warning
    init_pair(4, COLOR_RED, COLOR_BLACK);     // Error/High Load
}

Dashboard::~Dashboard() {
    endwin();  // Restore terminal settings
}

void Dashboard::updateStats(const SystemMetrics& metrics) {
    std::lock_guard<std::mutex> lock(m_uiMutex);
    m_lastMetrics = metrics;
    draw();
}

void Dashboard::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_uiMutex);

    m_logBuffer.push_back({level, message});
    if (m_logBuffer.size() > MAX_LOG_LINES) {
        m_logBuffer.pop_front();
    }
    draw();
}

void Dashboard::draw() {
    clear();  // Clear screen

    int height, width;
    getmaxyx(stdscr, height, width);

    // --- 1. Draw Title ---
    attron(COLOR_PAIR(1) | A_BOLD);
    const char* title = "=== SYS SENTINEL v1.0 ===";
    mvprintw(0, (width - 25) / 2, "%s", title);
    attroff(COLOR_PAIR(1) | A_BOLD);

    // --- 2. Draw Stats Box (Top Half) ---
    attron(COLOR_PAIR(1));
    mvprintw(2, 2, "SYSTEM METRICS");

    // FIX: Use simple '-' instead of ACS_HLINE for compatibility
    for (int i = 0; i < width; ++i) mvaddch(3, i, '-');
    attroff(COLOR_PAIR(1));

    // Calculate Values
    double ramUsed = m_lastMetrics.memoryUsed / (1024.0 * 1024.0 * 1024.0);
    double ramTotal = m_lastMetrics.memoryTotal / (1024.0 * 1024.0 * 1024.0);
    double diskFree = m_lastMetrics.diskFree / (1024.0 * 1024.0 * 1024.0);

    // FIX: CPU Bar Logic (More sensitive: 1 bar = 2%)
    // If load > 1% but bars calculation is 0, force at least 1 bar.
    int cpuBars = (int)(m_lastMetrics.cpuLoad / 2.0);
    if (m_lastMetrics.cpuLoad > 1.0 && cpuBars == 0) cpuBars = 1;
    if (cpuBars > 50) cpuBars = 50;  // Cap at 50 bars

    mvprintw(4, 2, "CPU Load:  [");
    for (int i = 0; i < 50; ++i) {
        if (i < cpuBars) {
            // Color logic: Green for low, Yellow for med, Red for high
            int color = 2;
            if (m_lastMetrics.cpuLoad > 50) color = 3;
            if (m_lastMetrics.cpuLoad > 80) color = 4;

            attron(COLOR_PAIR(color));
            addch('|');  // Simple pipe character is safer than blocks
            attroff(COLOR_PAIR(color));
        } else {
            addch(' ');
        }
    }
    printw("] %.2f%%", m_lastMetrics.cpuLoad);

    mvprintw(5, 2, "RAM Usage: %.2f GB / %.2f GB", ramUsed, ramTotal);
    mvprintw(6, 2, "Disk Free: %.2f GB", diskFree);

    // --- 3. Draw Log Box (Bottom Half) ---
    attron(COLOR_PAIR(1));
    mvprintw(8, 2, "EVENT LOGS");
    // Draw separator line
    for (int i = 0; i < width; ++i) mvaddch(9, i, '-');
    attroff(COLOR_PAIR(1));

    int y = 10;
    for (auto it = m_logBuffer.rbegin(); it != m_logBuffer.rend(); ++it) {
        if (y >= height - 1) break;

        int colorPair = 2;                                  // Default INFO (Green)
        if (it->first == LogLevel::WARNING) colorPair = 3;  // Yellow
        if (it->first == LogLevel::ERROR) colorPair = 4;    // Red

        attron(COLOR_PAIR(colorPair));
        // Print the log message
        mvprintw(y++, 2, "> %s", it->second.c_str());
        attroff(COLOR_PAIR(colorPair));
    }

    refresh();  // Apply changes
}