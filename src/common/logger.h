#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <mutex>

class Logger {
public:
    Logger(const std::string& logFileName = "server_log.txt") {
        logFile.open(logFileName, std::ios::out | std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file.\n";
        }
    }

    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        if (logFile.is_open()) {
            logFile << message << std::endl;
        }
        std::cout << message << std::endl;  // Optional: to print to console as well
    }

private:
    std::ofstream logFile;
    std::mutex logMutex;
};
