#include "HttpConfig.hpp"
#include <iostream>
#include <algorithm>

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
            parseServerAttribute(configLine, serverData);
        }
    }

    std::cout << "Fin du parsing du serveur. Nombre de locations: " << serverData.locations.size() << std::endl;

    validateServerConfiguration(serverData);
    parsedServers.push_back(serverData);
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
    } else if (attributeKey == "root") {
        serverData.root = attributeValue;
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

void HttpConfig::parseLocationConfig(std::istringstream& configStream, ServerConfig& serverData) {
    Location location;
    std::string configLine;
    bool isFirstLocation = true;

    std::cout << "Début du parsing des locations" << std::endl;

    while (std::getline(configStream, configLine)) {
        trimWhitespace(configLine);
        if (configLine.empty() || configLine[0] == '#') continue;
        
        std::cout << "Ligne de location en cours de traitement: " << configLine << std::endl;

        if (configLine.find("- path:") != std::string::npos || isFirstLocation) {
            if (!isFirstLocation) {
                std::cout << "Ajout d'une location complète. Path: " << location.path << std::endl;
                serverData.locations.push_back(location);
            }
            location = Location(); // Réinitialiser la location
            if (configLine.find("- path:") != std::string::npos) {
                location.path = configLine.substr(configLine.find(":") + 1);
            } else {
                location.path = "/"; // Pour la première location si elle n'a pas de "- path:"
            }
            trimWhitespace(location.path);
            std::cout << "Nouvelle location initialisée avec path: " << location.path << std::endl;
            isFirstLocation = false;
            continue;
        }

        size_t separatorPosition = configLine.find(": ");
        if (separatorPosition == std::string::npos) {
            if (configLine == "redirect:") {
                // Traitement de la redirection
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
                            location.redirect.code = std::atoi(redirectValue.c_str());
                        }
                    }
                    if (redirectLine.find("code:") != std::string::npos) break;
                }
            } else {
                std::cout << "Erreur: format d'attribut de location invalide: " << configLine << std::endl;
                throw std::runtime_error("Invalid location attribute format: " + configLine);
            }
        } else {
            std::string key = configLine.substr(0, separatorPosition);
            std::string value = configLine.substr(separatorPosition + 2);

            if (key == "methods") {
                location.methods = split(value.substr(1, value.length() - 2), ',');
                for (size_t i = 0; i < location.methods.size(); ++i) {
                    trimWhitespace(location.methods[i]);
                }
            } else if (key == "root") {
                location.root = value;
            } else if (key == "index") {
                location.index = value;
            } else if (key == "autoindex") {
                location.autoindex = (value == "on");
            } else if (key == "cgi_extensions") {
                location.cgiExtensions = split(value.substr(1, value.length() - 2), ',');
            } else if (key == "allow_uploads") {
                location.allowUploads = (value == "true");
            } else if (key == "upload_store") {
                location.uploadStore = value;
            } else if (key == "client_max_body_size") {
                location.clientMaxBodySize = parseBodySizeLimit(value);
            } else if (key == "handler") {
                location.handler = value;
            } else if (key == "requires_auth") {
                location.requiresAuth = (value == "true");
            } else if (key == "content_type") {
                location.contentType = value;
            } else if (key == "fastcgi_pass") {
                location.fastcgiPass = value;
            } else if (key == "fastcgi_index") {
                location.fastcgiIndex = value;
            } else if (key == "include") {
                location.include = value;
            }
        }
    }

    if (!location.path.empty()) {
        std::cout << "Ajout de la dernière location. Path: " << location.path << std::endl;
        serverData.locations.push_back(location);
    }

    std::cout << "Fin du parsing des locations. Nombre total: " << serverData.locations.size() << std::endl;
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
        std::cout << "Erreur: Aucune location définie pour le serveur " << serverData.serverName << std::endl;
        throw std::runtime_error("No locations defined for server");
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