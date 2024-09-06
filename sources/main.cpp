#include <string>
#include <iostream>
#include <fstream>
#include "Config.hpp"
#include "Location.hpp"
#include "Server.hpp"
#include "Socket.hpp"
#include "Client.hpp"
#include "Utils.hpp"

int main(int ac, char **av)
{
    if (ac != 2) {
        std::cout << "Binary must be: ./webserv [configuration file]" << std::endl;
        return (1);
    }
	const size_t buffer_size = 1024;
	ifstream settings(argv[1].c_str());
	Config config(settings);
	Server webserv(config.getFdConfig(), config.getLocation());
	webserv.handleRequest();
	return (0);
}