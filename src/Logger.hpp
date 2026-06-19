#pragma once
#include "precompiled.h"
#include <chrono>

class Logger {
public:
    enum class LogLevel {
        INFO,
        WARNING,
        ERROR
    };

    static void Init(const std::string& filename) {
        instance().log_file_.open(filename, std::ios::out | std::ios::app);
        if (!instance().log_file_.is_open()) {
            std::cerr << "Logger: failed to open log file: " << filename << std::endl;
        }
    }

    static void Log(const std::string& message, LogLevel level = LogLevel::INFO) {
        const char* level_str = "";
        switch (level) {
            case LogLevel::INFO:    level_str = "INFO";    break;
            case LogLevel::WARNING: level_str = "WARNING"; break;
            case LogLevel::ERROR:   level_str = "ERROR";   break;
        }

        const auto now = std::chrono::system_clock::now();
        const auto time_t = std::chrono::system_clock::to_time_t(now);
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::tm tm_buf{};
#ifdef _WIN32
        localtime_s(&tm_buf, &time_t);
#else
        localtime_r(&time_t, &tm_buf);
#endif

        char time_str[32];
        std::strftime(time_str, sizeof(time_str), "%H:%M:%S", &tm_buf);

        const std::string line = std::string("[") + time_str + "."
            + std::to_string(ms.count()) + "]["
            + level_str + "] " + message;

        if (instance().log_file_.is_open()) {
            instance().log_file_ << line << std::endl;
        }
        std::cout << line << std::endl;
    }

    static void Close() {
        instance().log_file_.close();
    }

private:
    Logger() = default;
    ~Logger() = default;

    static Logger& instance() {
        static Logger instance;
        return instance;
    }

    std::ofstream log_file_;
};
