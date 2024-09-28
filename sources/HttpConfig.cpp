#include "HttpConfig.hpp"

HttpConfig::HttpConfig() {}

HttpConfig::~HttpConfig() {}

HttpConfig::HttpConfig(const std::string& configPath) {
    loadConfigFromFile(configPath);
}

const std::vector<HttpConfig::ServerConfig>& HttpConfig::getParsedServers() const {
    return _parsedServers;
}

void HttpConfig::loadConfigFromFile(const std::string& configPath) {
    _configContent = readConfigFile(configPath);
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
    const std::string serverDelimiter = "- server:\n";
    size_t currentPosition = 0, nextServerPosition;

    while ((nextServerPosition = _configContent.find(serverDelimiter, currentPosition)) != std::string::npos) {
        if (currentPosition != 0) {
            rawServerConfigs.push_back(_configContent.substr(currentPosition, nextServerPosition - currentPosition));
        }
        currentPosition = nextServerPosition + serverDelimiter.length();
    }

    if (currentPosition < _configContent.length()) {
        rawServerConfigs.push_back(_configContent.substr(currentPosition));
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

    while (std::getline(configStream, configLine)) {
        trimWhitespace(configLine);
        if (configLine.empty() || configLine[0] == '#') continue;

        if (configLine == "error_pages:") {
            currentSection = "error_pages";
        } else if (configLine == "routes:") {
            currentSection = "routes";
        } else if (currentSection == "error_pages") {
            parseErrorPageConfig(configLine, serverData);
        } else if (currentSection == "routes") {
            parseRouteConfig(configLine, serverData);
        } else {
            parseServerAttribute(configLine, serverData);
        }
    }

    validateServerConfiguration(serverData);
    _parsedServers.push_back(serverData);
}

void HttpConfig::parseServerAttribute(const std::string& attributeLine, ServerConfig& serverData) {
    size_t separatorPosition = attributeLine.find(": ");
    if (separatorPosition == std::string::npos) {
        throw std::runtime_error("Invalid attribute format: " + attributeLine);
    }

    const std::string attributeKey = attributeLine.substr(0, separatorPosition);
    const std::string attributeValue = attributeLine.substr(separatorPosition + 2);

    if (attributeKey == "server_name") {
        serverData.serverName = attributeValue;
    } else if (attributeKey == "port") {
        serverData.port = parsePortNumber(attributeValue);
    } else if (attributeKey == "client_max_body_size") {
        serverData.clientMaxBodySize = parseBodySizeLimit(attributeValue);
    }
}

int HttpConfig::parsePortNumber(const std::string& portString) {
    if (!isAllDigits(portString)) {
        throw std::runtime_error("Port must consist exclusively of numbers");
    }
    int port = std::atoi(portString.c_str());
    if (port < 0 || port > 65535) {
        throw std::runtime_error("Port number out of valid range (0-65535)");
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

    return sizeLimit;
}

void HttpConfig::parseErrorPageConfig(const std::string& errorPageLine, ServerConfig& serverData) {
	size_t separatorPosition = errorPageLine.find(": ");
	if (separatorPosition == std::string::npos) {
		throw std::runtime_error("Invalid error_page format: " + errorPageLine);
	}

	std::string errorCodeString = errorPageLine.substr(0, separatorPosition);
	std::string errorPagePath = errorPageLine.substr(separatorPosition + 2);

	if (!isAllDigits(errorCodeString)) {
		throw std::runtime_error("Error code must be a number: " + errorCodeString);
	}

	int errorCode = std::atoi(errorCodeString.c_str());

	serverData.errorPages[errorCode] = errorPagePath;
}


void HttpConfig::parseRouteConfig(const std::string& routeLine, ServerConfig& serverData) {
    static std::string currentRoutePath;

    if (routeLine[0] == '-') {
        currentRoutePath = routeLine.substr(2);
        serverData.routes[currentRoutePath] = std::map<std::string, std::string>();
    } else {
        size_t separatorPosition = routeLine.find(": ");
        if (separatorPosition == std::string::npos) {
            throw std::runtime_error("Invalid route attribute format: " + routeLine);
        }

        const std::string routeKey = routeLine.substr(0, separatorPosition);
        const std::string routeValue = routeLine.substr(separatorPosition + 2);
        serverData.routes[currentRoutePath].insert(std::make_pair(routeKey, routeValue));
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
    if (serverData.routes.empty()) {
        throw std::runtime_error("No routes defined for server");
    }
}

void HttpConfig::trimWhitespace(std::string& str) {
    if (str.empty()) return;

    // Trim leading whitespace
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");

    if (first != std::string::npos && last != std::string::npos) {
        str = str.substr(first, last - first + 1);
    } else {
        str.clear();
    }
}

bool HttpConfig::isAllDigits(const std::string& str) {
    return std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun(isdigit))) == str.end();
}
