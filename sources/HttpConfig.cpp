#include "HttpConfig.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <set>
#include <sys/stat.h>

HttpConfig::HttpConfig() {}

HttpConfig::~HttpConfig() {}

HttpConfig::HttpConfig(const std::string& configPath) {
    loadConfigFromFile(configPath);
}

const std::vector<HttpConfig::ServerConfig>& HttpConfig::getParsedServers() const {
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
    std::vector<std::string> rawServerConfigs = splitServerConfigurations();
    for (std::vector<std::string>::const_iterator it = rawServerConfigs.begin(); it != rawServerConfigs.end(); ++it) {
        parseServerConfiguration(*it);
    }
}

std::vector<std::string> HttpConfig::splitServerConfigurations() {
    std::vector<std::string> rawServerConfigs;
    const std::string serverDelimiter = "- server:";
    size_t currentPosition = 0, nextServerPosition;

    while ((nextServerPosition = configContent.find(serverDelimiter, currentPosition)) != std::string::npos) {
        if (currentPosition != 0) {
            rawServerConfigs.push_back(configContent.substr(currentPosition, nextServerPosition - currentPosition));
        }
        currentPosition = nextServerPosition + serverDelimiter.length();
    }

    if (currentPosition < configContent.length()) {
        rawServerConfigs.push_back(configContent.substr(currentPosition));
    }

    if (rawServerConfigs.empty()) {
        throw std::runtime_error("Invalid configuration: no server blocks found");
    }

    return rawServerConfigs;
}
void HttpConfig::parseServerConfiguration(const std::string& serverConfig) {
    ServerConfig serverData;
    std::istringstream configStream(serverConfig);
    std::string configLine, currentSection;
    std::set<std::string> definedAttributes;

    std::cout << "Début du parsing d'un nouveau serveur" << std::endl;

    while (std::getline(configStream, configLine)) {
        trimWhitespace(configLine);
        if (configLine.empty() || configLine[0] == '#') continue;

        std::cout << "Ligne en cours de traitement: " << configLine << std::endl;

        if (configLine == "error_pages:") {
            currentSection = "errorPages";
        } else if (configLine == "locations:") {
            currentSection = "locations";
            std::cout << "Début de la section locations" << std::endl;
        } else if (currentSection == "errorPages") {
            parseErrorPageConfig(configLine, serverData);
        } else if (currentSection == "locations") {
            parseLocationConfig(configStream, serverData);
        } else {
            parseServerAttribute(configLine, serverData, definedAttributes);
        }
    }

    std::cout << "Fin du parsing du serveur. Nombre de locations: " << serverData.locations.size() << std::endl;

    validateServerConfiguration(serverData);
    parsedServers.push_back(serverData);
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
    if (port < 1024) {
        std::cout << "Warning: Using a privileged port (< 1024). This may require root privileges." << std::endl;
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

    if (!fileExists(errorPagePath)) {
        throw std::runtime_error("Error page file does not exist: " + errorPagePath);
    }

    serverData.errorPages[errorCode] = errorPagePath;
}

void HttpConfig::parseLocationConfig(std::istringstream& configStream, ServerConfig& serverData) {
    Location location;
    std::string configLine;
    bool isFirstLocation = true;
    std::set<std::string> locationPaths;

    std::cout << "Début du parsing des locations" << std::endl;

    while (std::getline(configStream, configLine)) {
        trimWhitespace(configLine);
        if (configLine.empty() || configLine[0] == '#') continue;
        
        std::cout << "Ligne de location en cours de traitement: " << configLine << std::endl;

        if (configLine.find("- path:") != std::string::npos || isFirstLocation) {
            if (!isFirstLocation) {
                validateLocation(location, serverData);
                serverData.locations.push_back(location);
            }
            location = Location();
            if (configLine.find("- path:") != std::string::npos) {
                location.path = configLine.substr(configLine.find(":") + 1);
            } else {
                location.path = "/"; // Pour la première location si elle n'a pas de "- path:"
            }
            trimWhitespace(location.path);
            if (location.path[0] != '/') {
                throw std::runtime_error("Location path must start with '/': " + location.path);
            }
            if (locationPaths.find(location.path) != locationPaths.end()) {
                throw std::runtime_error("Duplicate location path: " + location.path);
            }
            locationPaths.insert(location.path);
            std::cout << "Nouvelle location initialisée avec path: " << location.path << std::endl;
            isFirstLocation = false;
            continue;
        }

        size_t separatorPosition = configLine.find(": ");
        if (separatorPosition == std::string::npos) {
            if (configLine == "redirect:") {
                parseRedirect(configStream, location);
            } else {
                std::cout << "Erreur: format d'attribut de location invalide: " << configLine << std::endl;
                throw std::runtime_error("Invalid location attribute format: " + configLine);
            }
        } else {
            std::string key = configLine.substr(0, separatorPosition);
            std::string value = configLine.substr(separatorPosition + 2);
            parseLocationAttribute(key, value, location, serverData);
        }
    }

    if (!location.path.empty()) {
        validateLocation(location, serverData);
        serverData.locations.push_back(location);
    }

    std::cout << "Fin du parsing des locations. Nombre total: " << serverData.locations.size() << std::endl;
}

void HttpConfig::parseLocationAttribute(const std::string& key, const std::string& value, Location& location, const ServerConfig& serverData) {
    if (key == "methods") {
        std::vector<std::string> methodsVector = split(value.substr(1, value.length() - 2), ',');
        location.methods.clear();
        for (std::vector<std::string>::iterator it = methodsVector.begin(); it != methodsVector.end(); ++it) {
            std::string method = *it;
            trimWhitespace(method);
            method = toUpperCase(method);
            if (method == "GET" || method == "POST" || method == "DELETE") {
                location.methods.push_back(method);
            } else {
                throw std::runtime_error("Invalid HTTP method: " + method + ". Only GET, POST, and DELETE are allowed.");
            }
        }
        if (location.methods.empty()) {
            throw std::runtime_error("At least one valid HTTP method must be specified.");
        }
    } else if (key == "root") {
        location.root = value;
        if (!directoryExists(location.root)) {
            throw std::runtime_error("Location root directory does not exist or is not accessible: " + location.root);
        }
    } else if (key == "index") {
        location.index = value;
    } else if (key == "autoindex") {
        if (value != "on" && value != "off") {
            throw std::runtime_error("Invalid autoindex value. Must be 'on' or 'off'.");
        }
        location.autoindex = (value == "on");
    } else if (key == "cgi_extensions") {
        location.cgiExtensions = split(value.substr(1, value.length() - 2), ',');
        for (std::vector<std::string>::iterator it = location.cgiExtensions.begin(); it != location.cgiExtensions.end(); ++it) {
            if ((*it)[0] != '.') {
                throw std::runtime_error("CGI extension must start with a dot: " + *it);
            }
        }
    } else if (key == "allow_uploads") {
        if (value != "true" && value != "false") {
            throw std::runtime_error("Invalid allow_uploads value. Must be 'true' or 'false'.");
        }
        location.allowUploads = (value == "true");
    } else if (key == "upload_store") {
        location.uploadStore = value;
        if (!directoryExists(location.uploadStore)) {
            throw std::runtime_error("Upload store directory does not exist or is not accessible: " + location.uploadStore);
        }
    } else if (key == "client_max_body_size") {
        location.clientMaxBodySize = parseBodySizeLimit(value);
        if (location.clientMaxBodySize > serverData.clientMaxBodySize) {
            throw std::runtime_error("Location client_max_body_size exceeds server's limit");
        }
    } else if (key == "handler") {
        location.handler = value;
    } else if (key == "requires_auth") {
        if (value != "true" && value != "false") {
            throw std::runtime_error("Invalid requires_auth value. Must be 'true' or 'false'.");
        }
        location.requiresAuth = (value == "true");
    } else if (key == "content_type") {
        location.contentType = value;
    } else if (key == "fastcgi_pass") {
        location.fastcgiPass = value;
    } else if (key == "fastcgi_index") {
        location.fastcgiIndex = value;
    } else if (key == "include") {
        location.include = value;
	} else    if (key == "default_file") {
        location.defaultFile = value;
    } else {
        throw std::runtime_error("Unknown location attribute: " + key);
    }
}

void HttpConfig::validateLocation(const Location& location, const ServerConfig& serverData) {
    if (location.path.empty() || location.path[0] != '/') {
        throw std::runtime_error("Invalid location path: " + location.path);
    }

    if (location.methods.empty()) {
        throw std::runtime_error("No HTTP methods defined for location: " + location.path);
    }
    for (std::vector<std::string>::const_iterator it = location.methods.begin(); it != location.methods.end(); ++it) {
        if (*it != "GET" && *it != "POST" && *it != "DELETE") {
            throw std::runtime_error("Invalid HTTP method for location " + location.path + ": " + *it);
        }
    }

    if (location.clientMaxBodySize > serverData.clientMaxBodySize) {
        throw std::runtime_error("Location client_max_body_size exceeds server's limit for location: " + location.path);
    }

    if (!location.redirect.url.empty() && location.redirect.code == 0) {
        throw std::runtime_error("Incomplete redirect configuration for location: " + location.path);
    }
}

void HttpConfig::parseRedirect(std::istringstream& configStream, Location& location) {
    std::string redirectLine;
    while (std::getline(configStream, redirectLine)) {
        trimWhitespace(redirectLine);
        if (redirectLine.empty() || redirectLine[0] == '#') continue;
        size_t pos = redirectLine.find(": ");
        if (pos != std::string::npos) {
            std::string redirectKey = redirectLine.substr(0, pos);
            std::string redirectValue = redirectLine.substr(pos + 2);
            if (redirectKey == "url") {
                location.redirect.url = redirectValue;
            } else if (redirectKey == "code") {
                int code = std::atoi(redirectValue.c_str());
                if (code != 301 && code != 302 && code != 303 && code != 307 && code != 308) {
                    throw std::runtime_error("Invalid redirect code: " + redirectValue);
                }
                location.redirect.code = code;
            }
        }
        if (redirectLine.find("code:") != std::string::npos) break;
    }
    if (location.redirect.url.empty() || location.redirect.code == 0) {
        throw std::runtime_error("Incomplete redirect configuration");
    }
}

void HttpConfig::validateServerConfiguration(const ServerConfig& serverData) {
    if (serverData.serverName.empty()) {
        throw std::runtime_error("Server name is missing");
    }
    if (serverData.port == 0) {
        throw std::runtime_error("Port is missing or invalid");
    }
    if (serverData.clientMaxBodySize == 0) {
        throw std::runtime_error("Client max body size is missing or invalid");
    }
    if (serverData.locations.empty()) {
        throw std::runtime_error("No locations defined for server: " + serverData.serverName);
    }
    if (!directoryExists(serverData.root)) {
        throw std::runtime_error("Server root directory does not exist or is not accessible: " + serverData.root);
    }
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
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    if (first != std::string::npos && last != std::string::npos) {
        str = str.substr(first, (last - first + 1));
    } else {
        str.clear();
    }
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
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    return std::find(location.cgiExtensions.begin(), location.cgiExtensions.end(), "." + extension) != location.cgiExtensions.end();
}

bool HttpConfig::shouldListDirectory(const Location& location, const std::string& path) {
    return location.autoindex && directoryExists(path);
}