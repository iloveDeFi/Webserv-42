#pragma once
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <set>

class HttpConfig {
public:
    struct Location {
        std::string path;
        std::vector<std::string> methods;
        std::string root;
        std::string index;
        bool autoindex;
        std::vector<std::string> cgiExtensions;
        bool allowUploads;
        std::string uploadStore;
        size_t clientMaxBodySize;
        std::string handler;
        bool requiresAuth;
        std::string contentType;
        struct {
            std::string url;
            int code;
        } redirect;
        std::string fastcgiPass;
        std::string fastcgiIndex;
        std::string include;
    };

    struct ServerConfig {
        std::string serverName;
        int port;
        size_t clientMaxBodySize;
        std::string root;
        std::map<int, std::string> errorPages;
        std::vector<Location> locations;
    };

    HttpConfig();
    HttpConfig(const std::string& configPath);
    ~HttpConfig();

    void loadConfigFromFile(const std::string& configPath);
    const std::vector<ServerConfig>& getParsedServers() const;

private:
    std::vector<ServerConfig> parsedServers;
    std::string configContent;

    std::string readConfigFile(const std::string& path);
    void parseConfigurationFile();
    std::vector<std::string> splitServerConfigurations();
    void parseServerConfiguration(const std::string& serverConfig);
    void parseServerAttribute(const std::string& attributeLine, ServerConfig& serverData);
    int parsePortNumber(const std::string& portString);
    size_t parseBodySizeLimit(const std::string& sizeString);
    void parseErrorPageConfig(const std::string& errorPageLine, ServerConfig& serverData);
    void parseLocationConfig(std::istringstream& configStream, ServerConfig& serverData);
    void validateServerConfiguration(const ServerConfig& serverData);
    static void trimWhitespace(std::string& str);
    static bool isAllDigits(const std::string& str);
    static std::vector<std::string> split(const std::string& s, char delimiter);
};