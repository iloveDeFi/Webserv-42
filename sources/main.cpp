#include <string>
#include <iostream>
<<<<<<< HEAD
#include <fstream>
#include "Config.hpp"
#include "Location.hpp"
#include "Server.hpp"
#include "Socket.hpp"
#include "Client.hpp"
#include "Utils.hpp"
=======
#include "HttpRequest.hpp"

>>>>>>> feature/http-request

int main(int ac, char **av)
{
    if (ac != 2) {
        std::cout << "Binary must be: ./webserv [configuration file]" << std::endl;
        return (1);
    }
<<<<<<< HEAD
	const size_t buffer_size = 1024;
	ifstream settings(argv[1].c_str());
	Config config(settings);
	Server webserv(config.getFdConfig(), config.getLocation());
	webserv.handleRequest();
	return (0);
}
=======

    HttpRequest simulatedRequest;

    simulatedRequest.setMethod("GET");
    simulatedRequest.setURI("/index.html");
    simulatedRequest.setHTTPVersion("HTTP/1.1");

    std::map<std::string, std::string> headers;
    headers["Host"] = "localhost";
    headers["Connection"] = "keep-alive";
    simulatedRequest.setHeaders(headers);
    simulatedRequest.setBody("");
    simulatedRequest.setIsChunked(false);

    std::cout << "Method: " << simulatedRequest.getMethod() << std::endl;
    std::cout << "URI: " << simulatedRequest.getURI() << std::endl;
    std::cout << "HTTP Version: " << simulatedRequest.getHTTPVersion() << std::endl;

    return 0;
}
>>>>>>> feature/http-request
