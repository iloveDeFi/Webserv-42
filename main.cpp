#include <string>
#include <iostream>
#include <fstream>
#include "Config.hpp"
#include "Location.hpp"
#include "MngmtServers.hpp"
#include "Socket.hpp"
#include "Client.hpp"
#include "Utils.hpp"
#include "HttpController.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

int main(int ac, char **av)
{
    if (ac != 2) {
        std::cout << "Binary must be: ./webserv [configuration file]" << std::endl;
        return (1);
    }
	  // Loading configuration
    std::ifstream settings(av[1]);
    if (!settings.is_open()) {
        std::cerr << "Error opening configuration file." << std::endl;
        return (1);
    }

	Config config(settings);
	ManagementServer webserv(config.getFdConfig(), config.getLocations());
    
    // My Loop to handle all client requests
    while (true) {
        try {
            // Helene : TO DO : accept client connexions
            Client client = webserv.acceptNewClients(std::vector<int> &clientFds, fd_set &readFds); 
            if (client.isConnected()) {

                // Alex : TO DO : read and parse client http request 
                std::string requestStr = client.readRequest(); // Remplacez ceci par votre méthode de lecture
                HttpRequest httpRequest(requestStr);

                // Baptiste : TO DO : Handle request and generation of my Http response
                RequestController requestController;
                HttpResponse httpResponse = requestController.handleRequest(httpRequest);

                // Helene : TO DO : Send response to client
                client.sendResponse(/*httpResponse*/); 
            }
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
	return (0);
}