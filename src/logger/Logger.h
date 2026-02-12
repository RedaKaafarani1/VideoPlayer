#pragma once

#include <fstream>
#include <string>
#include <format>
#include <utility>
#include <chrono>

class Logger {
public:
    //Singleton, delete non-authorized operations
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    //use a single instance throughout the project
    static Logger& instance(const std::string& filename) {
        static Logger logger(filename);
        return logger;
    }

    enum class LogLevel {
        DEBUG,
        INFO,
        WARN,
        ERROR
    };
    
    static constexpr Logger::LogLevel MIN_LOG_LEVEL = Logger::LogLevel::DEBUG;

    constexpr std::string_view logLevelToString(LogLevel level) noexcept
    {
        switch (level)
        {
            case LogLevel::DEBUG : return "DEBUG";
            case LogLevel::INFO  : return "INFO";
            case LogLevel::WARN  : return "WARN";
            case LogLevel::ERROR : return "ERROR";
        }
        return "UNKNOWN";
    }

    template<typename... Args>
    void debug(const std::format_string<Args...>& fmt, Args&&... args)
    {
       log(LogLevel::DEBUG, fmt, std::forward<Args>(args)...); 
    }
    
    template<typename... Args>
    void info(const std::format_string<Args...>& fmt, Args&&... args)
    {
       log(LogLevel::INFO, fmt, std::forward<Args>(args)...); 
    }

    template<typename... Args>
    void warn(const std::format_string<Args...>& fmt, Args&&... args)
    {
       log(LogLevel::WARN, fmt, std::forward<Args>(args)...); 
    }

    template<typename... Args>
    void error(const std::format_string<Args...>& fmt, Args&&... args)
    {
       log(LogLevel::ERROR, fmt, std::forward<Args>(args)...); 
    }
private:
    Logger(std::string fileName) : _fileName(std::move(fileName)) { initializeLogger(); }
    void initializeLogger();
    
    template<typename... Args>
    void log(LogLevel level, const std::format_string<Args...>& fmt, Args&&... args)
    {
        if (_logFile.is_open() && level >= MIN_LOG_LEVEL)
        {
            std::chrono::zoned_time localTime{std::chrono::current_zone(), std::chrono::system_clock::now()};
            auto timeStr = std::format("{:%FT%T}", localTime); 
            _logFile <<timeStr<<":"
                     <<"[" << logLevelToString(level) << "]"
                     << std::format(fmt, std::forward<Args>(args)...) << "\n";
            _logFile.flush();
        }
    }
    
    std::string _fileName;
    std::ofstream _logFile; 
};
