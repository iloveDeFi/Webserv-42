
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
    std::ifstream configFile(configPath.c_str());
    if (!configFile.is_open()) {
        std::cerr << "Impossible d'ouvrir le fichier de configuration : " << configPath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(configFile, line)) {
        line = _trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        if (line == "- server:") {
            ServerConfig server;
            if (!_parseServerBlock(configFile, server)) {
                std::cerr << "Erreur lors du parsing du bloc serveur." << std::endl;
                return false;
            }
            _servers.push_back(server);
        }
    }

    return true;
}

bool HttpConfig::_parseServerBlock(std::ifstream& file, ServerConfig& server) {
    std::string line;
    int indentLevel = 4; // On suppose une indentation de 4 espaces

    while (std::getline(file, line)) {
        line = _trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        if (line.find("server_name:") == 0) {
            server.serverName = _trim(line.substr(12));
        } else if (line.find("port:") == 0) {
            server.port = atoi(_trim(line.substr(5)).c_str());
        } else if (line.find("client_max_body_size:") == 0) {
            server.clientMaxBodySize = _parseSize(_trim(line.substr(21)));
        } else if (line.find("root:") == 0) {
            server.root = _trim(line.substr(5));
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
            server.errorPages[code] = path;
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
                location.maxUploadSize = _parseSize(value);
            }
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

size_t HttpConfig::_parseSize(const std::string& str) {
    size_t multiplier = 1;
    std::string number = str;
    char lastChar = str[str.length() - 1];
    if (lastChar == 'K' || lastChar == 'k') {
        multiplier = 1024;
        number = str.substr(0, str.length() - 1);
    } else if (lastChar == 'M' || lastChar == 'm') {
        multiplier = 1024 * 1024;
        number = str.substr(0, str.length() - 1);
    }
    return static_cast<size_t>(atoi(number.c_str())) * multiplier;
}

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
        methods.push_back(method);
    }
    return methods;
}
