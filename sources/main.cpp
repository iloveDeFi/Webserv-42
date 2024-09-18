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
