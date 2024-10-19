#include <string>
#include <iostream>
#include <fstream>
#include "MngmtServers.hpp"
#include "HttpConfig.hpp"
#include "Socket.hpp"
#include "Client.hpp"
#include "Config.hpp"
#include "Utils.hpp"
#include "HttpController.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

int main(int ac, char **av)
{
	// std::string rawRequestGet = 
    // "GET /index.html HTTP/1.1\r\n"
    // "Host: localhost:8081\r\n"
    // "User-Agent: curl/7.81.0\r\n"
    // "Accept: */*\r\n"
    // "\r\n";

	// testRequest(rawRequestGet);
    if (ac != 2)
    {
        std::cout << "Binary must be: ./webserv [configuration file]" << std::endl;
        return (1);
    }

    // Charger la configuration
    // std::ifstream settings(av[1]);
    // if (!settings.is_open()) {
    //     std::cerr << "Error opening configuration file." << std::endl;
    //     return (1);
    // }

    HttpConfig config(av[1]);
    ManagementServer webserv(config);

    try
    {
        webserv.handleRequest();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return (1);
    }

    return (0);
}