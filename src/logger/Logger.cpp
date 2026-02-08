#include "Logger.h"

void Logger::initializeLogger()
{
    _logFile.open(_fileName, std::ios::out | std::ios::app);
    if (!_logFile.is_open())
    {
        //we do not want to crash the app for logging, output an error and continue
        std::cerr << "Could not open log file " << _fileName << " for writing\n";
    }
}

