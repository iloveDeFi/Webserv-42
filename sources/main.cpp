#include "../includes/HttpConfig.hpp"
#include <iostream>
#include <stdexcept>

// Fonction pour afficher les détails d'un serveur
void printServerDetails(const HttpConfig::ServerConfig& server) {
    std::cout << "Server Name: " << server.serverName << std::endl;
    std::cout << "Port: " << server.port << std::endl;
    std::cout << "Max Body Size: " << server.clientMaxBodySize << " bytes" << std::endl;
    
    std::cout << "Error Pages:" << std::endl;
    for (std::map<int, std::string>::const_iterator it = server.errorPages.begin(); 
         it != server.errorPages.end(); ++it) {
        std::cout << "  " << it->first << ": " << it->second << std::endl;
    }
    
    std::cout << "Routes:" << std::endl;
    for (std::map<std::string, std::map<std::string, std::string> >::const_iterator it = server.routes.begin(); 
         it != server.routes.end(); ++it) {
        std::cout << "  " << it->first << std::endl;
        for (std::map<std::string, std::string>::const_iterator innerIt = it->second.begin(); 
             innerIt != it->second.end(); ++innerIt) {
            std::cout << "    " << innerIt->first << ": " << innerIt->second << std::endl;
        }
    }
    std::cout << std::endl;
}

int main() {
    const std::string configFilename = "config.yaml";
    
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
