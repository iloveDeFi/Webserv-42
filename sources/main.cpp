#include "../includes/HttpConfig.hpp"
#include <iostream>
#include <stdexcept>

// Fonction pour afficher les détails d'une location
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
    
    if (!location.cgi_extensions.empty()) {
        std::cout << "    CGI Extensions: ";
        for (size_t i = 0; i < location.cgi_extensions.size(); ++i) {
            std::cout << location.cgi_extensions[i];
            if (i < location.cgi_extensions.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
    
    std::cout << "    Allow Uploads: " << (location.allow_uploads ? "true" : "false") << std::endl;
    
    if (!location.upload_store.empty()) {
        std::cout << "    Upload Store: " << location.upload_store << std::endl;
    }
    
    if (location.client_max_body_size > 0) {
        std::cout << "    Client Max Body Size: " << location.client_max_body_size << " bytes";
        if (location.client_max_body_size >= 1024 * 1024) {
            std::cout << " (" << (location.client_max_body_size / (1024 * 1024)) << "M)";
        } else if (location.client_max_body_size >= 1024) {
            std::cout << " (" << (location.client_max_body_size / 1024) << "K)";
        }
        std::cout << std::endl;
    }
    
    if (!location.handler.empty()) {
        std::cout << "    Handler: " << location.handler << std::endl;
    }
    
    std::cout << "    Requires Auth: " << (location.requires_auth ? "true" : "false") << std::endl;
    
    if (!location.content_type.empty()) {
        std::cout << "    Content Type: " << location.content_type << std::endl;
    }
    
    if (!location.redirect.url.empty()) {
        std::cout << "    Redirect:" << std::endl;
        std::cout << "      URL: " << location.redirect.url << std::endl;
        std::cout << "      Code: " << location.redirect.code << std::endl;
    }
    
    if (!location.fastcgi_pass.empty()) {
        std::cout << "    FastCGI Pass: " << location.fastcgi_pass << std::endl;
    }
    
    if (!location.fastcgi_index.empty()) {
        std::cout << "    FastCGI Index: " << location.fastcgi_index << std::endl;
    }
    
    if (!location.include.empty()) {
        std::cout << "    Include: " << location.include << std::endl;
    }
}

// Fonction pour afficher les détails d'un serveur
void printServerDetails(const HttpConfig::ServerConfig& server) {
    std::cout << "Server Name: " << server.server_name << std::endl;
    std::cout << "Port: " << server.port << std::endl;
    std::cout << "Max Body Size: " << server.client_max_body_size << " bytes" << std::endl;
    if (!server.root.empty()) std::cout << "Root: " << server.root << std::endl;
    
    std::cout << "Error Pages:" << std::endl;
    for (std::map<int, std::string>::const_iterator it = server.error_pages.begin(); 
         it != server.error_pages.end(); ++it) {
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
        // Charger et parser la configuration
        HttpConfig config(configFilename);
        
        // Récupérer les configurations de serveurs parsées
        const std::vector<HttpConfig::ServerConfig>& servers = config.getParsedServers();
        
        // Afficher les détails de chaque serveur
        std::cout << "Parsed " << servers.size() << " server configurations:" << std::endl << std::endl;
        for (size_t i = 0; i < servers.size(); ++i) {
            std::cout << "Server " << (i + 1) << ":" << std::endl;
            printServerDetails(servers[i]);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}