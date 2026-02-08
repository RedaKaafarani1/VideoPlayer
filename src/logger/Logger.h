#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <format>

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

    template<typename... Args>
    void log(const std::format_string<Args...>& fmt, Args&&... args)
    {
        if (_logFile.is_open())
        {
            _logFile << std::format(fmt, std::forward<Args>(args)...) << "\n";
            _logFile.flush();
        }
    }

private:
    Logger(std::string fileName) : _fileName(std::move(fileName)) { initializeLogger(); }
    void initializeLogger();
    
    std::string _fileName;
    std::ofstream _logFile;
};
