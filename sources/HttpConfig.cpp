
#include "HttpConfig.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <cctype>

HttpConfig::HttpConfig(const std::string& configPath) 
{
    loadConfigFromFile(configPath);
}

HttpConfig::~HttpConfig() {}

const std::vector<ServerConfig>& HttpConfig::getServers() const {
    return _servers;
}

bool HttpConfig::loadConfigFromFile(const std::string& configPath) {
    std::ifstream file(configPath.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open configuration file: " + configPath);
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = _trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        if (line == "- server:") {
            ServerConfig server;
            if (!_parseServerBlock(file, server)) {
                throw std::runtime_error("Error parsing configuration bloc: ");
                return false;
            }
            _servers.push_back(server);
        }
    }
    if (file.bad()) {
        throw std::runtime_error("Error reading configuration file: " + configPath);
        return false;
    }

    return true;
}

bool HttpConfig::_parseServerBlock(std::ifstream& file, ServerConfig& server) {
    std::string line;
    int indentLevel = 4; //indentation de 4 espaces

    while (std::getline(file, line)) {
        line = _trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        if (line.find("server_name:") == 0) {
            server.serverName = _trim(line.substr(12));
        } else if (line.find("port:") == 0) {
            server.port = _parsePortNumber(_trim(line.substr(5)).c_str());
        } else if (line.find("client_max_body_size:") == 0) {
            server.clientMaxBodySize = _parseBodySizeLimit(_trim(line.substr(21)));
        } else if (line.find("root:") == 0) {
            server.root = _trim(line.substr(5));
            if (!_directoryExists(server.root)) {
            throw std::runtime_error("Server root directory does not exist or is not accessible: " + server.root);
        }
        } else if (line == "error_pages:") {
            if (!_parseErrorPages(file, server, indentLevel))
                return false;
        } else if (line == "locations:") {
            if (!_parseLocations(file, server, indentLevel))
                return false;
        } else if (line == "- server:") {
            // Reculer d'une ligne si un nouveau serveur est détecté
            file.seekg(-static_cast<int>(line.length()) - 1, std::ios_base::cur);
            break;
        }
    }
    return true;
}

bool HttpConfig::_parseErrorPages(std::ifstream& file, ServerConfig& server, int indentLevel) {
    std::string line;
    int currentIndent;
    while (std::getline(file, line)) {
        currentIndent = _countLeadingSpaces(line);
        if (currentIndent < indentLevel)
            break;

        line = _trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string codeStr = _trim(line.substr(0, colonPos));
            std::string path = _trim(line.substr(colonPos + 1));
            int code = atoi(codeStr.c_str());
            if (code < 400 || code >= 600) {
                throw std::runtime_error("Invalid HTTP error code (must be between 400 and 599): " + line);
            }
            if (path.empty()) {
                throw std::runtime_error("Error page path cannot be empty for error code: " + line);
            }

            if (!_fileExists(path)) {
                throw std::runtime_error("Error page file does not exist: " + line);
            }
            server.errorPages[code] = path;
        } else {
        throw std::runtime_error("Invalid error_page format: " + line);
            return false;
        }
    }
    // Reculer d'une ligne pour le prochain parsing
    file.seekg(-static_cast<int>(line.length()) - 1, std::ios_base::cur);
    return true;
}

bool HttpConfig::_parseLocations(std::ifstream& file, ServerConfig& server, int indentLevel) {
    std::string line;
    int currentIndent;
    while (std::getline(file, line)) {
        currentIndent = _countLeadingSpaces(line);
        if (currentIndent < indentLevel)
            break;

        line = _trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        if (line.find("- path:") == 0) {
            Location location;
            location.path = _trim(line.substr(7));
            if (!_parseLocationBlock(file, location, indentLevel + 2))
                return false;
            server.locations.push_back(location);
        }
    }
    // Reculer d'une ligne pour le prochain parsing
    file.seekg(-static_cast<int>(line.length()) - 1, std::ios_base::cur);
    return true;
}

bool HttpConfig::_parseLocationBlock(std::ifstream& file, Location& location, int indentLevel) {
    std::string line;
    int currentIndent;
    while (std::getline(file, line)) {
        currentIndent = _countLeadingSpaces(line);
        if (currentIndent < indentLevel)
            break;

        line = _trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = _trim(line.substr(0, colonPos));
            std::string value = _trim(line.substr(colonPos + 1));

            if (key == "methods") {
                location.methods = _splitMethods(value);
            } else if (key == "handler") {
                location.handler = value;
            } else if (key == "allow_uploads") {
                location.allowUploads = (value == "true");
            } else if (key == "content_type") {
                location.contentType = value;
            } else if (key == "max_upload_size") {
                location.maxUploadSize = _parseBodySizeLimit(value);
            }
        } else {
            throw std::runtime_error("Invalid location infos format: " + line);
            return false;
        }
    }
    // Reculer d'une ligne pour le prochain parsing
    file.seekg(-static_cast<int>(line.length()) - 1, std::ios_base::cur);
    return true;
}

std::string HttpConfig::_trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

int HttpConfig::_countLeadingSpaces(const std::string& str) {
    int count = 0;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == ' ')
            ++count;
        else
            break;
    }
    return count;
}

// size_t HttpConfig::_parseSize(const std::string& str) {
//     size_t multiplier = 1;
//     std::string number = str;
//     char lastChar = str[str.length() - 1];
//     if (lastChar == 'K' || lastChar == 'k') {
//         multiplier = 1024;
//         number = str.substr(0, str.length() - 1);
//     } else if (lastChar == 'M' || lastChar == 'm') {
//         multiplier = 1024 * 1024;
//         number = str.substr(0, str.length() - 1);
//     }
//     return static_cast<size_t>(atoi(number.c_str())) * multiplier;
// }

std::vector<std::string> HttpConfig::_splitMethods(const std::string& str) {
    std::vector<std::string> methods;
    size_t start = str.find('[');
    size_t end = str.find(']');
    if (start == std::string::npos || end == std::string::npos || end <= start)
        return methods;

    std::string methodsStr = str.substr(start + 1, end - start - 1);

    std::istringstream iss(methodsStr);
    std::string method;
    while (std::getline(iss, method, ',')) {
        method = _trim(method);
        std::string upperMethod = method;
        for (size_t i = 0; i < upperMethod.length(); ++i) {
            std::string upperMethod = _toUpperCase(method);
        }

        if (upperMethod == "GET" || upperMethod == "POST" || upperMethod == "DELETE") {
            methods.push_back(upperMethod);
        } else {
            throw std::runtime_error("Invalid HTTP method: " + method + ". Only GET, POST, and DELETE are allowed.");
        }
    }
    return methods;
}



int HttpConfig::_parsePortNumber(const std::string& portString) {
    if (!_isAllDigits(portString)) {
        throw std::runtime_error("Port must consist exclusively of numbers");
    }
    int port = std::atoi(portString.c_str());
    if (port <= 0 || port > 65535) {
        throw std::runtime_error("Port number out of valid range (1-65535)");
    }
    if (port < 1024) {
    }
    return port;
}

bool HttpConfig::_isAllDigits(const std::string& str) {
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
        if (!std::isdigit(*it)) {
            return false;
        }
    }
    return !str.empty();
}

size_t HttpConfig::_parseBodySizeLimit(const std::string& sizeString) {
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

    if (!_isAllDigits(numericPart)) {
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

bool HttpConfig::_fileExists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFREG) != 0;
}

std::string HttpConfig::_toUpperCase(const std::string& str) {
    std::string result = str;
    for (std::string::iterator it = result.begin(); it != result.end(); ++it) {
        *it = std::toupper(static_cast<unsigned char>(*it));
    }
    return result;
}

bool HttpConfig::_directoryExists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
}

// HttpConfig::~HttpConfig() {}

// HttpConfig::HttpConfig(const std::string& configPath) {
//     loadConfigFromFile(configPath);
// }

// std::vector<HttpConfig::ServerConfig>& HttpConfig::getParsedServers() {
//     return parsedServers;
// }

// void HttpConfig::loadConfigFromFile(const std::string& configPath) {
//     configContent = readConfigFile(configPath);
//     parseConfigurationFile();
// }

// std::string HttpConfig::readConfigFile(const std::string& configPath) {
//     std::ifstream file(configPath.c_str());
//     if (!file.is_open()) {
//         throw std::runtime_error("Cannot open configuration file: " + configPath);
//     }

//     std::stringstream buffer;
//     std::string line;
//     while (std::getline(file, line)) {
//         trimWhitespace(line);
//         if (!line.empty() && line[0] != '#') {
//             buffer << line << '\n';
//         }
//     }

//     if (file.bad()) {
//         throw std::runtime_error("Error reading configuration file: " + configPath);
//     }

//     return buffer.str();
// }

// void HttpConfig::parseConfigurationFile() {
//     std::istringstream configStream(configContent);
//     while (parseServerConfiguration(configStream)) {}
//     if (parsedServers.empty()) {
//         throw std::runtime_error("No valid server configuration found");
//     }
// }

// bool HttpConfig::parseServerConfiguration(std::istringstream& configStream) {
//     ServerConfig serverData;
//     std::string configLine, currentSection;
//     std::set<std::string> definedAttributes;

//     while (std::getline(configStream, configLine)) {
//         trimWhitespace(configLine);
//         if (configLine.empty() || configLine[0] == '#') {
//             continue;
//         }

//         if (configLine == "- server:") {
//             if (!serverData.serverName.empty()) {
//                 validateServerConfiguration(serverData);
//                 parsedServers.push_back(serverData);
//                 return true;
//             }
//             continue;
//         }

//         if (configLine == "error_pages:") {
//             currentSection = "errorPages";
//         } else if (configLine == "locations:") {
//             currentSection = "locations";
//             parseLocationConfig(configStream, serverData);
//         } else if (currentSection == "errorPages") {
//             parseErrorPageConfig(configLine, serverData);
//         } else {
//             parseServerAttribute(configLine, serverData, definedAttributes);
//         }
//     }

//     if (!serverData.serverName.empty()) {
//         validateServerConfiguration(serverData);
//         parsedServers.push_back(serverData);
//     }

//     return false;
// }

// void HttpConfig::parseServerAttribute(const std::string& attributeLine, ServerConfig& serverData, std::set<std::string>& definedAttributes) {
//     size_t separatorPosition = attributeLine.find(": ");
//     if (separatorPosition == std::string::npos) {
//         throw std::runtime_error("Invalid attribute format: " + attributeLine);
//     }

//     const std::string attributeKey = attributeLine.substr(0, separatorPosition);
//     const std::string attributeValue = attributeLine.substr(separatorPosition + 2);

//     if (definedAttributes.find(attributeKey) != definedAttributes.end()) {
//         throw std::runtime_error("Duplicate attribute: " + attributeKey);
//     }
//     definedAttributes.insert(attributeKey);

//     if (attributeKey == "server_name") {
//         serverData.serverName = attributeValue;
//     } else if (attributeKey == "port") {
//         serverData.port = parsePortNumber(attributeValue);
//     } else if (attributeKey == "client_max_body_size") {
//         serverData.clientMaxBodySize = parseBodySizeLimit(attributeValue);
//     } else if (attributeKey == "root") {
//         serverData.root = attributeValue;
//         if (!directoryExists(serverData.root)) {
//             throw std::runtime_error("Server root directory does not exist or is not accessible: " + serverData.root);
//         }
//     } else {
//         throw std::runtime_error("Unknown server attribute: " + attributeKey);
//     }
// }

// int HttpConfig::parsePortNumber(const std::string& portString) {
//     if (!isAllDigits(portString)) {
//         throw std::runtime_error("Port must consist exclusively of numbers");
//     }
//     int port = std::atoi(portString.c_str());
//     if (port <= 0 || port > 65535) {
//         throw std::runtime_error("Port number out of valid range (1-65535)");
//     }
//     if (port < 1024) {
//     }
//     return port;
// }

// size_t HttpConfig::parseBodySizeLimit(const std::string& sizeString) {
//     size_t multiplier = 1;
//     std::string numericPart = sizeString;

//     char lastChar = sizeString[sizeString.length() - 1];
//     if (lastChar == 'k' || lastChar == 'K') {
//         multiplier = 1024;
//         numericPart = sizeString.substr(0, sizeString.length() - 1);
//     } else if (lastChar == 'm' || lastChar == 'M') {
//         multiplier = 1024 * 1024;
//         numericPart = sizeString.substr(0, sizeString.length() - 1);
//     }

//     if (!isAllDigits(numericPart)) {
//         throw std::runtime_error("Invalid client_max_body_size format");
//     }

//     size_t sizeLimit = static_cast<size_t>(std::atol(numericPart.c_str())) * multiplier;
//     if (sizeLimit == 0) {
//         throw std::runtime_error("client_max_body_size must be greater than 0");
//     }
//     if (sizeLimit > 1024 * 1024 * 1024) { // 1 GB limit
//         throw std::runtime_error("client_max_body_size exceeds maximum allowed (1 GB)");
//     }

//     return sizeLimit;
// }

// void HttpConfig::parseErrorPageConfig(const std::string& errorPageLine, ServerConfig& serverData) {
//     size_t separatorPosition = errorPageLine.find(": ");
//     if (separatorPosition == std::string::npos) {
//         throw std::runtime_error("Invalid error_page format: " + errorPageLine);
//     }

//     std::string errorCodeString = errorPageLine.substr(0, separatorPosition);
//     std::string errorPagePath = errorPageLine.substr(separatorPosition + 2);

//     if (errorCodeString.empty() || !isAllDigits(errorCodeString)) {
//         throw std::runtime_error("Error code must be a valid number: " + errorCodeString);
//     }

//     int errorCode = std::atoi(errorCodeString.c_str());

//     if (errorCode < 400 || errorCode >= 600) {
//         throw std::runtime_error("Invalid HTTP error code (must be between 400 and 599): " + errorCodeString);
//     }

//     if (errorPagePath.empty()) {
//         throw std::runtime_error("Error page path cannot be empty for error code: " + errorCodeString);
//     }

//     if (!fileExists(errorPagePath)) {
//         throw std::runtime_error("Error page file does not exist: " + errorPagePath);
//     }

//     serverData.errorPages[errorCode] = errorPagePath;
// }

// void HttpConfig::parseLocationConfig(std::istringstream& configStream, ServerConfig& serverData) {
//     Location location;
//     std::string configLine;
//     bool isFirstLocation = true;
//     std::set<std::string> locationPaths;


// 	while (std::getline(configStream, configLine)) {
//         trimWhitespace(configLine);
//         if (configLine.empty() || configLine[0] == '#') continue;
         

//         if (configLine == "- server:" || configLine.find("server_name:") != std::string::npos) {
//             configStream.seekg(-static_cast<int>(configLine.length()) - 1, std::ios::cur); // Revenir en arrière pour que cette ligne soit relue
//             break;
//         }

//         if (configLine.find("- path:") != std::string::npos) {
//             if (!isFirstLocation) {
//                 serverData.locations.push_back(location);
//             }
//             location = Location();
//             location.path = configLine.substr(configLine.find(":") + 1);
//             trimWhitespace(location.path);
//             isFirstLocation = false;
//         }
//     }

//     if (!location.path.empty()) {
//         serverData.locations.push_back(location);
//     }

// }
// void HttpConfig::parseLocationAttribute(const std::string& key, const std::string& value, Location& location, const ServerConfig& serverData) {
//     if (key == "methods") {
//         std::vector<std::string> methodsVector = split(value.substr(1, value.length() - 2), ',');
//         location.methods.clear();
//         for (std::vector<std::string>::iterator it = methodsVector.begin(); it != methodsVector.end(); ++it) {
//             std::string method = *it;
//             trimWhitespace(method);
//             method = toUpperCase(method);
//             if (method == "GET" || method == "POST" || method == "DELETE") {
//                 location.methods.push_back(method);
//             } else {
//                 throw std::runtime_error("Invalid HTTP method: " + method + ". Only GET, POST, and DELETE are allowed.");
//             }
//         }
//         if (location.methods.empty()) {
//             throw std::runtime_error("At least one valid HTTP method must be specified.");
//         }
//     } else if (key == "root") {
//         location.root = value;
//         if (!directoryExists(location.root)) {
//             throw std::runtime_error("Location root directory does not exist or is not accessible: " + location.root);
//         }
//     } else if (key == "index") {
//         location.index = value;
//     } else if (key == "autoindex") {
//         if (value != "on" && value != "off") {
//             throw std::runtime_error("Invalid autoindex value. Must be 'on' or 'off'.");
//         }
//         location.autoindex = (value == "on");
//     } else if (key == "cgi_extensions") {
//         location.cgiExtensions = split(value.substr(1, value.length() - 2), ',');
//         for (std::vector<std::string>::iterator it = location.cgiExtensions.begin(); it != location.cgiExtensions.end(); ++it) {
//             if ((*it)[0] != '.') {
//                 throw std::runtime_error("CGI extension must start with a dot: " + *it);
//             }
//         }
//     } else if (key == "allow_uploads") {
//         if (value != "true" && value != "false") {
//             throw std::runtime_error("Invalid allow_uploads value. Must be 'true' or 'false'.");
//         }
//         location.allowUploads = (value == "true");
//     } else if (key == "upload_store") {
//         location.uploadStore = value;
//         if (!directoryExists(location.uploadStore)) {
//             throw std::runtime_error("Upload store directory does not exist or is not accessible: " + location.uploadStore);
//         }
//     } else if (key == "client_max_body_size") {
//         location.clientMaxBodySize = parseBodySizeLimit(value);
//         if (location.clientMaxBodySize > serverData.clientMaxBodySize) {
//             throw std::runtime_error("Location client_max_body_size exceeds server's limit");
//         }
//     } else if (key == "handler") {
//         location.handler = value;
//     } else if (key == "requires_auth") {
//         if (value != "true" && value != "false") {
//             throw std::runtime_error("Invalid requires_auth value. Must be 'true' or 'false'.");
//         }
//         location.requiresAuth = (value == "true");
//     } else if (key == "content_type") {
//         location.contentType = value;
//     } else if (key == "include") {
//         location.include = value;
// 	} else if (key == "default_file") {
//         location.defaultFile = value;
//     } else if (key == "cgi_extensions") {
//         location.cgiExtensions = split(value.substr(1, value.length() - 2), ',');
//         for (std::vector<std::string>::iterator it = location.cgiExtensions.begin(); it != location.cgiExtensions.end(); ++it) {
//             trimWhitespace(*it);
//             if ((*it)[0] != '.') {
//                 throw std::runtime_error("CGI extension must start with a dot: " + *it);
//             }
//         }
//     } else if (key == "cgi_handler") {
//         location.cgiHandler = value;
//     }
// 	 else {
//         throw std::runtime_error("Unknown location attribute: " + key);
//     }
// }

// void HttpConfig::validateLocation(const Location& location, const ServerConfig& serverData) {

//     if (location.path.empty() || location.path[0] != '/') {
//         throw std::runtime_error("Invalid location path: " + location.path);
//     }

//     if (!location.redirect.url.empty()) {
//         if (location.redirect.code == 0) {
//             throw std::runtime_error("Incomplete redirect configuration for location: " + location.path);
//         }
//         return;
//     }


//     if (location.methods.empty()) {
//         throw std::runtime_error("No HTTP methods defined for non-redirect location: " + location.path);
//     }

//     for (std::vector<std::string>::const_iterator it = location.methods.begin(); it != location.methods.end(); ++it) {
//         if (*it != "GET" && *it != "POST" && *it != "DELETE") {
//             throw std::runtime_error("Invalid HTTP method for location " + location.path + ": " + *it);
//         }
//     }

//     if (location.clientMaxBodySize > serverData.clientMaxBodySize) {
//         throw std::runtime_error("Location client_max_body_size exceeds server's limit for location: " + location.path);
//     }
// }

// void HttpConfig::parseRedirect(std::istringstream& configStream, Location& location) {
//     std::string redirectLine;
//     while (std::getline(configStream, redirectLine)) {
//         trimWhitespace(redirectLine);
//         if (redirectLine.empty() || redirectLine[0] == '#') continue;
//         size_t pos = redirectLine.find(": ");
//         if (pos != std::string::npos) {
//             std::string redirectKey = redirectLine.substr(0, pos);
//             std::string redirectValue = redirectLine.substr(pos + 2);
//             if (redirectKey == "url") {
//                 location.redirect.url = redirectValue;
//             } else if (redirectKey == "code") {
//                 int code = std::atoi(redirectValue.c_str());
//                 if (code != 301 && code != 302 && code != 303 && code != 307 && code != 308) {
//                     throw std::runtime_error("Invalid redirect code: " + redirectValue);
//                 }
//                 location.redirect.code = code;
//             }
//         }
//         if (redirectLine.find("code:") != std::string::npos) break;
//     }
//     if (location.redirect.url.empty() || location.redirect.code == 0) {
//         throw std::runtime_error("Incomplete redirect configuration for location: " + location.path);
//     }
// }

// void HttpConfig::validateServerConfiguration(const ServerConfig& serverData) {
//     if (serverData.serverName.empty()) {
//         throw std::runtime_error("Server name is missing");
//     }
//     if (serverData.port == 0) {
//         throw std::runtime_error("Port is missing or invalid");
//     }
//     if (serverData.clientMaxBodySize == 0) {
//         throw std::runtime_error("Client max body size is missing or invalid");
//     }
//     if (serverData.locations.empty()) {
//         throw std::runtime_error("No locations defined for server: " + serverData.serverName);
//     }
//     if (!directoryExists(serverData.root)) {
//         throw std::runtime_error("Server root directory does not exist or is not accessible: " + serverData.root);
//     }
// }

// bool HttpConfig::directoryExists(const std::string& path) {
//     struct stat info;
//     if (stat(path.c_str(), &info) != 0) {
//         return false;
//     }
//     return (info.st_mode & S_IFDIR) != 0;
// }

// bool HttpConfig::fileExists(const std::string& path) {
//     struct stat info;
//     if (stat(path.c_str(), &info) != 0) {
//         return false;
//     }
//     return (info.st_mode & S_IFREG) != 0;
// }

// std::string HttpConfig::toUpperCase(const std::string& str) {
//     std::string result = str;
//     for (std::string::iterator it = result.begin(); it != result.end(); ++it) {
//         *it = std::toupper(static_cast<unsigned char>(*it));
//     }
//     return result;
// }

// void HttpConfig::trimWhitespace(std::string& str) {
//     size_t first = str.find_first_not_of(" \t\n\r");
//     size_t last = str.find_last_not_of(" \t\n\r");
//     if (first != std::string::npos && last != std::string::npos) {
//         str = str.substr(first, (last - first + 1));
//     } else {
//         str.clear();
//     }
// }

// bool HttpConfig::isAllDigits(const std::string& str) {
//     for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
//         if (!std::isdigit(*it)) {
//             return false;
//         }
//     }
//     return !str.empty();
// }

// std::vector<std::string> HttpConfig::split(const std::string& s, char delimiter) {
//     std::vector<std::string> tokens;
//     std::string token;
//     std::istringstream tokenStream(s);
//     while (std::getline(tokenStream, token, delimiter)) {
//         trimWhitespace(token);
//         if (!token.empty()) {
//             tokens.push_back(token);
//         }
//     }
//     return tokens;
// }

// bool HttpConfig::isCgiScript(const Location& location, const std::string& filename) {
//     if (location.cgiExtensions.empty()) {
//         return false;
//     }
//     std::string extension = filename.substr(filename.find_last_of(".") + 1);
//     return std::find(location.cgiExtensions.begin(), location.cgiExtensions.end(), "." + extension) != location.cgiExtensions.end();
// }

// bool HttpConfig::shouldListDirectory(const Location& location, const std::string& path) {
//     return location.autoindex && directoryExists(path);
// }