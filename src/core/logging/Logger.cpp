#include "Logger.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace cad {
namespace core {

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::initialize(const std::string& file_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    file_path_ = file_path;
}

void Logger::setMinLevel(Level level) {
    std::lock_guard<std::mutex> lock(mutex_);
    min_level_ = level;
}

void Logger::log(Level level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (level < min_level_) {
        return;
    }

    const std::string line = timestamp() + " [" + levelToString(level) + "] " + message + "\n";
    std::cerr << line;
    if (!file_path_.empty()) {
        std::ofstream stream(file_path_, std::ios::app);
        if (stream.is_open()) {
            stream << line;
        }
    }
}

std::string Logger::levelToString(Level level) const {
    switch (level) {
        case Level::Debug:
            return "DEBUG";
        case Level::Info:
            return "INFO";
        case Level::Warn:
            return "WARN";
        case Level::Error:
            return "ERROR";
        default:
            return "INFO";
    }
}

std::string Logger::timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm_time{};
#if defined(_WIN32)
    localtime_s(&tm_time, &time);
#else
    localtime_r(&time, &tm_time);
#endif
    std::ostringstream out;
    out << std::put_time(&tm_time, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

}  // namespace core
}  // namespace cad
