#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

class Logger
{
private:
    std::ofstream _logFile;
    std::string getCurrentTime();

public:
    Logger(const std::string &filename);
    ~Logger();

    void log(const std::string &message);
    void logError(const std::string &errorMessage);
    void logRequest(const std::string &method, const std::string &uri, int statusCode);
};

#endif