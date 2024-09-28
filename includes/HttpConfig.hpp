#pragma once
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <string>
#include <map>

class HttpConfig {
public:
    struct ServerConfig {
        std::string serverName;
        int port;
        size_t clientMaxBodySize;
        std::map<int, std::string> errorPages;
        std::map<std::string, std::map<std::string, std::string> > routes;
    };

    HttpConfig();
    explicit HttpConfig(const std::string& configPath);
    ~HttpConfig();

    void loadConfigFromFile(const std::string& configPath);
    const std::vector<ServerConfig>& getParsedServers() const;

private:
    std::vector<ServerConfig> _parsedServers;
    std::string _configContent;

    std::string readConfigFile(const std::string& path);
    void parseConfigurationFile();
    std::vector<std::string> splitServerConfigurations();
    void parseServerConfiguration(const std::string& serverConfig);
    void parseServerAttribute(const std::string& attributeLine, ServerConfig& serverData);
    int parsePortNumber(const std::string& portString);
    size_t parseBodySizeLimit(const std::string& sizeString);
    void parseErrorPageConfig(const std::string& errorPageLine, ServerConfig& serverData);
    void parseRouteConfig(const std::string& routeLine, ServerConfig& serverData);
    void validateServerConfiguration(const ServerConfig& serverData);
    static void trimWhitespace(std::string& str);
    static bool isAllDigits(const std::string& str);
};
