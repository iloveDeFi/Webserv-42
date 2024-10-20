#include "Logger.hpp"
#include <ctime>

// Constructor that opens the log file
Logger::Logger(const std::string &filename)
{
    _logFile.open(filename.c_str(), std::ios::app); // Append mode
    if (!_logFile.is_open())
    {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
}

// Destructor that closes the log file
Logger::~Logger()
{
    if (_logFile.is_open())
    {
        _logFile.close();
    }
}

// Get current time as a string for the log entries
std::string Logger::getCurrentTime()
{
    std::time_t now = std::time(0);
    char buf[80];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %X", std::localtime(&now));
    return std::string(buf);
}

// General log function
void Logger::log(const std::string &message)
{
    if (_logFile.is_open())
    {
        _logFile << "[" << getCurrentTime() << "] " << message << std::endl;
    }
}

// Specific function to log errors
void Logger::logError(const std::string &errorMessage)
{
    log("ERROR: " + errorMessage);
}

// Function to log details of HTTP requests
void Logger::logRequest(const std::string &method, const std::string &uri, int statusCode)
{
    log("REQUEST: Method=" + method + ", URI=" + uri + ", Status Code=" + std::to_string(statusCode));
}