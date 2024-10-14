#pragma once
#ifndef HTTPCONFIG_HPP
# define HTTPCONFIG_HPP
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <cctype>
#include <vector>


struct Location {
    std::string path;
    std::vector<std::string> methods;
    std::string handler;
    bool allowUploads;
    std::string contentType;
    size_t maxUploadSize;

    Location() : allowUploads(false), maxUploadSize(0) {}
};

struct ServerConfig {
    std::string serverName;
    int port;
    size_t clientMaxBodySize;
    std::string root;
    std::map<int, std::string> errorPages;
    std::vector<Location> locations;

    ServerConfig() : port(0), clientMaxBodySize(0) {}
};

class HttpConfig {
public:
    HttpConfig(const std::string& configPath);
    ~HttpConfig();
    const std::vector<ServerConfig>& getServers() const;

private:
    std::vector<ServerConfig> _servers;
    
    bool loadConfigFromFile(const std::string& configPath);

    // Fonctions utilitaires pour le parsing
    std::string _trim(const std::string& str);
    int _countLeadingSpaces(const std::string& str);
    size_t _parseSize(const std::string& str);
    std::vector<std::string> _splitMethods(const std::string& str);

    // Méthodes de parsing
    bool _parseServerBlock(std::ifstream& file, ServerConfig& server);
    bool _parseErrorPages(std::ifstream& file, ServerConfig& server, int indentLevel);
    bool _parseLocations(std::ifstream& file, ServerConfig& server, int indentLevel);
    bool _parseLocationBlock(std::ifstream& file, Location& location, int indentLevel);
};

#endif


// class HttpConfig {
// public:
//     struct Location {
//         std::string path;
//         std::vector<std::string> methods;
//         std::string root;
//         std::string index;
//         bool autoindex;
//         std::vector<std::string> cgiExtensions;
//         bool allowUploads;
//         std::string uploadStore;
//         size_t clientMaxBodySize;
//         std::string handler;
//         bool requiresAuth;
//         std::string contentType;
//         struct {
//             std::string url;
//             int code;
//         } redirect;
//         std::string fastcgiPass;
//         std::string fastcgiIndex;
//         std::string include;
// 		std::string defaultFile;

//     };

//     struct ServerConfig {
//         std::string serverName;
//         int port;
//         size_t clientMaxBodySize;
//         std::string root;
//         std::map<int, std::string> errorPages;
//         std::vector<Location> locations;
//     };

//     //HttpConfig();
//     HttpConfig(const std::string& configPath);
//     ~HttpConfig();

//     void loadConfigFromFile(const std::string& configPath);
//     std::vector<ServerConfig>& getParsedServers();
// 	static bool isCgiScript(const Location& location, const std::string& filename);
//     static bool shouldListDirectory(const Location& location, const std::string& path);

// private:
//     std::vector<ServerConfig> parsedServers;
//     std::string configContent;

//     std::string readConfigFile(const std::string& path);
//     void parseConfigurationFile();
//     std::vector<std::string> splitServerConfigurations();
//     void parseServerConfiguration(const std::string& serverConfig);
//     void parseServerAttribute(const std::string& attributeLine, ServerConfig& serverData, std::set<std::string>& definedAttributes);
//     int parsePortNumber(const std::string& portString);
//     size_t parseBodySizeLimit(const std::string& sizeString);
//     void parseErrorPageConfig(const std::string& errorPageLine, ServerConfig& serverData);
//     void parseLocationConfig(std::istringstream& configStream, ServerConfig& serverData);
//     void parseLocationAttribute(const std::string& key, const std::string& value, Location& location, const ServerConfig& serverData);
//     void parseRedirect(std::istringstream& configStream, Location& location);
//     void validateServerConfiguration(const ServerConfig& serverData);
//     void validateLocation(const Location& location, const ServerConfig& serverData);
//     static bool directoryExists(const std::string& path);
//     static bool fileExists(const std::string& path);
//     static std::string toUpperCase(const std::string& str);
//     static void trimWhitespace(std::string& str);
//     static bool isAllDigits(const std::string& str);
//     static std::vector<std::string> split(const std::string& s, char delimiter);
// };