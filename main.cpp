#include <string>
#include <iostream>
#include <fstream>
#include "MngmtServers.hpp"
#include "HttpConfig.hpp"
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
	//std::ifstream settings(av[1]);
	HttpConfig config(av[1]);
	ManagementServer webserv(config);
	webserv.handleRequest();
	return (0);
}