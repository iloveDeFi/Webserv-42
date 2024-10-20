#include "Logger.hpp"

Logger *Logger::_instance = nullptr;

Logger::Logger(const std::string &filename)
{
    _logFile.open(filename.c_str(), std::ios::app);
    if (!_logFile.is_open())
    {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
}

Logger::~Logger()
{
    if (_logFile.is_open())
    {
        _logFile.close();
    }
}

Logger &Logger::getInstance(const std::string &filename)
{
    if (_instance == nullptr)
    {
        _instance = new Logger(filename);
    }
    return *_instance;
}

std::string Logger::getCurrentTime()
{
    std::time_t now = std::time(0);
    char buf[80];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %X", std::localtime(&now));
    return std::string(buf);
}

void Logger::log(const std::string &message)
{
    if (_logFile.is_open())
    {
        _logFile << "[" << getCurrentTime() << "] " << message << std::endl;
    }
}

void Logger::logError(const std::string &errorMessage)
{
    log("ERROR: " + errorMessage);
}

void Logger::logRequest(const std::string &method, const std::string &uri, int statusCode)
{
    log("REQUEST: Method=" + method + ", URI=" + uri + ", Status Code=" + to_string(statusCode));
}