#include "../includes/HttpConfig.hpp"
#include <iostream>
#include <stdexcept>

void printLocationDetails(const HttpConfig::Location& location) {
    std::cout << "    Path: " << location.path << std::endl;
    
    if (!location.methods.empty()) {
        std::cout << "    Methods: ";
        for (size_t i = 0; i < location.methods.size(); ++i) {
            std::cout << location.methods[i];
            if (i < location.methods.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
    
    if (!location.root.empty()) {
        std::cout << "    Root: " << location.root << std::endl;
    }
    
    if (!location.index.empty()) {
        std::cout << "    Index: " << location.index << std::endl;
    }
    
    std::cout << "    Autoindex: " << (location.autoindex ? "on" : "off") << std::endl;
    
    if (!location.cgiExtensions.empty()) {
        std::cout << "    CGI Extensions: ";
        for (size_t i = 0; i < location.cgiExtensions.size(); ++i) {
            std::cout << location.cgiExtensions[i];
            if (i < location.cgiExtensions.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
    
    std::cout << "    Allow Uploads: " << (location.allowUploads ? "true" : "false") << std::endl;
    
    if (!location.uploadStore.empty()) {
        std::cout << "    Upload Store: " << location.uploadStore << std::endl;
    }
    
    if (location.clientMaxBodySize > 0) {
        std::cout << "    Client Max Body Size: " << location.clientMaxBodySize << " bytes";
        if (location.clientMaxBodySize >= 1024 * 1024) {
            std::cout << " (" << (location.clientMaxBodySize / (1024 * 1024)) << "M)";
        } else if (location.clientMaxBodySize >= 1024) {
            std::cout << " (" << (location.clientMaxBodySize / 1024) << "K)";
        }
        std::cout << std::endl;
    }
    
    if (!location.handler.empty()) {
        std::cout << "    Handler: " << location.handler << std::endl;
    }
    
    std::cout << "    Requires Auth: " << (location.requiresAuth ? "true" : "false") << std::endl;
    
    if (!location.contentType.empty()) {
        std::cout << "    Content Type: " << location.contentType << std::endl;
    }
    
    if (!location.redirect.url.empty()) {
        std::cout << "    Redirect:" << std::endl;
        std::cout << "      URL: " << location.redirect.url << std::endl;
        std::cout << "      Code: " << location.redirect.code << std::endl;
    }
    
    if (!location.include.empty()) {
        std::cout << "    Include: " << location.include << std::endl;
    }

	if (!location.defaultFile.empty()) {
        std::cout << "    Default File: " << location.defaultFile << std::endl;
    }
}

void printServerDetails(const HttpConfig::ServerConfig& server) {
    std::cout << "Server Name: " << server.serverName << std::endl;
    std::cout << "Port: " << server.port << std::endl;
    std::cout << "Max Body Size: " << server.clientMaxBodySize << " bytes" << std::endl;
    if (!server.root.empty()) std::cout << "Root: " << server.root << std::endl;
    
    std::cout << "Error Pages:" << std::endl;
    for (std::map<int, std::string>::const_iterator it = server.errorPages.begin(); 
         it != server.errorPages.end(); ++it) {
        std::cout << "  " << it->first << ": " << it->second << std::endl;
    }
    
    std::cout << "Locations:" << std::endl;
    for (size_t i = 0; i < server.locations.size(); ++i) {
        std::cout << "  Location " << (i + 1) << ":" << std::endl;
        printLocationDetails(server.locations[i]);
    }
    std::cout << std::endl;
}

int main() {
    const std::string configFilename = "config1.yaml";
    
    try {
        std::cout << "Début du parsing du fichier de configuration: " << configFilename << std::endl;
        HttpConfig config(configFilename);
        const std::vector<HttpConfig::ServerConfig>& servers = config.getParsedServers();
        
        std::cout << "Nombre total de serveurs parsés: " << servers.size() << std::endl;
        
        for (size_t i = 0; i < servers.size(); ++i) {
            std::cout << "\nDétails du serveur " << (i + 1) << ":" << std::endl;
            printServerDetails(servers[i]);
            
            // Tests des locations
            // ...
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Erreur: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}