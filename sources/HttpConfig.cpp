#include "HttpConfig.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <set>
#include <sstream>
#include <sys/stat.h>

// Constructeur et destructeur
HttpConfig::~HttpConfig() {}

HttpConfig::HttpConfig(const std::string& configPath) {
    loadConfigFromFile(configPath);
}

std::vector<HttpConfig::ServerConfig>& HttpConfig::getParsedServers() {
    return parsedServers;
}

void HttpConfig::loadConfigFromFile(const std::string& configPath) {
    configContent = readConfigFile(configPath);
    parseConfigurationFile();
}

std::string HttpConfig::readConfigFile(const std::string& configPath) {
    std::ifstream file(configPath.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open configuration file: " + configPath);
    }

    std::stringstream buffer;
    std::string line;
    while (std::getline(file, line)) {
        trimWhitespace(line);
        if (!line.empty() && line[0] != '#') {
            buffer << line << '\n';
        }
    }

    if (file.bad()) {
        throw std::runtime_error("Error reading configuration file: " + configPath);
    }

    return buffer.str();
}

void HttpConfig::parseConfigurationFile() {
    std::istringstream configStream(configContent);
    while (parseServerConfiguration(configStream)) {}
    if (parsedServers.empty()) {
        throw std::runtime_error("No valid server configuration found");
    }
}

bool HttpConfig::parseServerConfiguration(std::istringstream& configStream) {
    ServerConfig serverData;
    std::string configLine, currentSection;
    std::set<std::string> definedAttributes;

    while (std::getline(configStream, configLine)) {
        trimWhitespace(configLine);
        if (configLine.empty() || configLine[0] == '#') {
            continue;
        }

        if (configLine == "- server:") {
            if (!serverData.serverName.empty()) {
                validateServerConfiguration(serverData);
                parsedServers.push_back(serverData);
                serverData = ServerConfig();
                definedAttributes.clear();
                return true;
            }
            continue;
        }

        if (configLine == "error_pages:") {
            currentSection = "errorPages";
        } else if (configLine == "locations:") {
            currentSection = "locations";
            parseLocationConfig(configStream, serverData);
        } else if (currentSection == "errorPages") {
            parseErrorPageConfig(configLine, serverData);
        } else {
            parseServerAttribute(configLine, serverData, definedAttributes);
        }
    }

    if (!serverData.serverName.empty()) {
        validateServerConfiguration(serverData);
        parsedServers.push_back(serverData);
    }

    return false;
}

void HttpConfig::parseLocationConfig(std::istringstream& configStream, ServerConfig& serverData) {
    Logger &logger = Logger::getInstance("server.log");

    Location location;
    std::string configLine;
    bool isFirstLocation = true;

    while (std::getline(configStream, configLine)) {
        trimWhitespace(configLine);
        if (configLine.empty() || configLine[0] == '#') continue;

        if (configLine == "- server:" || configLine.find("server_name:") != std::string::npos) {
            configStream.seekg(-static_cast<int>(configLine.length()) - 1, std::ios::cur);
            break;
        }

        if (configLine.find("- path:") != std::string::npos) {
            if (!isFirstLocation) {
                validateLocation(location, serverData);
                serverData.locations.push_back(location);
            }
            location = Location();
            location.path = configLine.substr(configLine.find(":") + 1);
            trimWhitespace(location.path);
            isFirstLocation = false;
            continue;
        }

        size_t separatorPosition = configLine.find(": ");
        if (separatorPosition != std::string::npos) {
            std::string key = configLine.substr(0, separatorPosition);
            std::string value = configLine.substr(separatorPosition + 2);
            trimWhitespace(key);
            trimWhitespace(value);

            if (key == "redirect") {
                parseRedirect(configStream, location);
            } else {
                parseLocationAttribute(key, value, location, serverData);
            }
        } else {
            throw std::runtime_error("Invalid location attribute format: " + configLine);
        }
    }

    if (!location.path.empty()) {
        validateLocation(location, serverData);
        serverData.locations.push_back(location);
    }
    logger.log("Location url + code " + location.redirect.url);
}

void HttpConfig::parseServerAttribute(const std::string& attributeLine, ServerConfig& serverData, std::set<std::string>& definedAttributes) {
    size_t separatorPosition = attributeLine.find(": ");
    if (separatorPosition == std::string::npos) {
        throw std::runtime_error("Invalid attribute format: " + attributeLine);
    }

    const std::string attributeKey = attributeLine.substr(0, separatorPosition);
    const std::string attributeValue = attributeLine.substr(separatorPosition + 2);

    if (definedAttributes.find(attributeKey) != definedAttributes.end()) {
        throw std::runtime_error("Duplicate attribute: " + attributeKey);
    }
    definedAttributes.insert(attributeKey);

    if (attributeKey == "server_name") {
        serverData.serverName = attributeValue;
    } else if (attributeKey == "port") {
        serverData.port = parsePortNumber(attributeValue);
    } else if (attributeKey == "client_max_body_size") {
        serverData.clientMaxBodySize = parseBodySizeLimit(attributeValue);
    } else if (attributeKey == "root") {
        serverData.root = attributeValue;
        if (!directoryExists(serverData.root)) {
            throw std::runtime_error("Server root directory does not exist or is not accessible: " + serverData.root);
        }
    } else {
        throw std::runtime_error("Unknown server attribute: " + attributeKey);
    }
}

int HttpConfig::parsePortNumber(const std::string& portString) {
    if (!isAllDigits(portString)) {
        throw std::runtime_error("Port must consist exclusively of numbers");
    }
    int port = std::atoi(portString.c_str());
    if (port <= 0 || port > 65535) {
        throw std::runtime_error("Port number out of valid range (1-65535)");
    }
    return port;
}

size_t HttpConfig::parseBodySizeLimit(const std::string& sizeString) {
    size_t multiplier = 1;
    std::string numericPart = sizeString;

    char lastChar = sizeString[sizeString.length() - 1];
    if (lastChar == 'k' || lastChar == 'K') {
        multiplier = 1024;
        numericPart = sizeString.substr(0, sizeString.length() - 1);
    } else if (lastChar == 'm' || lastChar == 'M') {
        multiplier = 1024 * 1024;
        numericPart = sizeString.substr(0, sizeString.length() - 1);
    }

    if (!isAllDigits(numericPart)) {
        throw std::runtime_error("Invalid client_max_body_size format");
    }

    size_t sizeLimit = static_cast<size_t>(std::atol(numericPart.c_str())) * multiplier;
    if (sizeLimit == 0) {
        throw std::runtime_error("client_max_body_size must be greater than 0");
    }
    if (sizeLimit > 1024 * 1024 * 1024) { // 1 GB limit
        throw std::runtime_error("client_max_body_size exceeds maximum allowed (1 GB)");
    }

    return sizeLimit;
}

void HttpConfig::parseErrorPageConfig(const std::string& errorPageLine, ServerConfig& serverData) {
    size_t separatorPosition = errorPageLine.find(": ");
    if (separatorPosition == std::string::npos) {
        throw std::runtime_error("Invalid error_page format: " + errorPageLine);
    }

    std::string errorCodeString = errorPageLine.substr(0, separatorPosition);
    std::string errorPagePath = errorPageLine.substr(separatorPosition + 2);

    if (errorCodeString.empty() || !isAllDigits(errorCodeString)) {
        throw std::runtime_error("Error code must be a valid number: " + errorCodeString);
    }

    int errorCode = std::atoi(errorCodeString.c_str());

    if (errorCode < 400 || errorCode >= 600) {
        throw std::runtime_error("Invalid HTTP error code (must be between 400 and 599): " + errorCodeString);
    }

    if (errorPagePath.empty()) {
        throw std::runtime_error("Error page path cannot be empty for error code: " + errorCodeString);
    }

    std::string fullPath = (errorPagePath[0] == '/') ? (serverData.root + errorPagePath) : (serverData.root + "/" + errorPagePath);

    if (!fileExists(fullPath)) {
        throw std::runtime_error("Error page file does not exist: " + fullPath);
    }

    serverData.errorPages[errorCode] = errorPagePath;
}

bool HttpConfig::directoryExists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
}

bool HttpConfig::fileExists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFREG) != 0;
}

std::string HttpConfig::toUpperCase(const std::string& str) {
    std::string result = str;
    for (std::string::iterator it = result.begin(); it != result.end(); ++it) {
        *it = std::toupper(static_cast<unsigned char>(*it));
    }
    return result;
}

void HttpConfig::trimWhitespace(std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    size_t end = str.find_last_not_of(" \t\n\r");
    str = (start != std::string::npos && end != std::string::npos) ? str.substr(start, end - start + 1) : "";
}

bool HttpConfig::isAllDigits(const std::string& str) {
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
        if (!std::isdigit(*it)) {
            return false;
        }
    }
    return !str.empty();
}

std::vector<std::string> HttpConfig::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        trimWhitespace(token);
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

bool HttpConfig::isCgiScript(const Location& location, const std::string& filename) {
    if (location.cgiExtensions.empty()) {
        return false;
    }
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    return std::find(location.cgiExtensions.begin(), location.cgiExtensions.end(), "." + extension) != location.cgiExtensions.end();
}

bool HttpConfig::shouldListDirectory(const Location& location, const std::string& path) {
    return location.autoindex && directoryExists(path);
}
