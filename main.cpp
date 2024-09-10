#include <string>
#include <iostream>
#include <fstream>
#include "Config.hpp"
#include "Location.hpp"
#include "MngmtServers.hpp"
#include "Socket.hpp"
#include "Client.hpp"
#include "Utils.hpp"

int main(int ac, char **av)
{
    if (ac != 2) {
        std::cout << "Binary must be: ./webserv [configuration file]" << std::endl;
        return (1);
    }
	std::ifstream settings(av[1]);
	Config config(settings);
	ManagementServer webserv(config.getFdConfig(), config.getLocations());
	webserv.handleRequest();
	return (0);
}