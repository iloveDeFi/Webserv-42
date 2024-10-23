#include "Client.hpp"

Client::Client(int fd, const struct sockaddr_in &address)
    : _socket(fd), _address(address), _request(""), _response() {}

Client::~Client() {}

void Client::readRequest(const std::string &rawData)
{
    _request = HttpRequest(rawData);
/*     std::cout << "Method: " << _request.getMethod() << std::endl;
    std::cout << "URI: " << _request.getURI() << std::endl;
    std::cout << "Version: " << _request.getHTTPVersion() << std::endl; */
}

void Client::processRequest(const _server &serverInfo)
{
    HttpResponse response;
    std::string uri = _request.getURI();
    std::string method = _request.getMethod();

    Logger &logger = Logger::getInstance("server.log");

    const HttpConfig::Location *bestMatch = NULL;

    // Find the location with the longest matching prefix
    for (size_t i = 0; i < serverInfo._locations.size(); ++i)
    {
        const HttpConfig::Location &location = serverInfo._locations[i];

        if (uri == location.path)
        {
            bestMatch = &location;
            break;
        }
    }

    if (bestMatch != NULL)
    {
        const HttpConfig::Location &location = *bestMatch;
        std::cout << "Matched Location Path: " << location.path << ", Handler: " << location.handler << std::endl;

        if (_request.getMethod() == "GET")
        {
            GetRequestHandler getHandler(location, serverInfo._root);
            getHandler.handle(_request, response);
        }
        else if (_request.getMethod() == "POST")
        {
            PostRequestHandler postHandler(location, serverInfo._root);
            postHandler.handle(_request, response);
        }
        else if (_request.getMethod() == "DELETE")
        {
            DeleteRequestHandler deleteHandler(location, serverInfo._root);
            deleteHandler.handle(_request, response);
        }
        else
        {
            UnknownRequestHandler unknownHandler(location, serverInfo._root);
            unknownHandler.handle(_request, response);
        }

        int statusCode = response.getStatusCode();
        logger.logRequest(method, uri, statusCode);
        if (statusCode >= 400)
        {
            logger.logError("Request resulted in error: " + to_string(statusCode));
        }
    }
    else
    {
        response.setStatusCode(404);
        response.setBody("404 Not Found");
        response.setHeader("Content-Type", "text/plain");
        response.ensureContentLength();
        logger.logError("404 Not Found for URI: " + uri);
    }

    _response = response;
}


void Client::sendResponse()
{
    std::string response = _response.toString();
    const char *data = response.c_str();
    size_t total = response.size();
    size_t sent = 0;
    ssize_t n;

    while (sent < total)
    {
        n = send(_socket, data + sent, total - sent, 0);
        if (n == -1)
        {
            if (errno == EINTR)
            {
                continue; // Réessayer si l'envoi a été interrompu par un signal
            }
            else
            {
                throw std::runtime_error("Error sending response: " + std::string(strerror(errno)));
            }
        }
        sent += n; // Ajouter le nombre d'octets envoyés avec succès
    }
}

void Client::setHttpRequest(const HttpRequest &request)
{
    _request = request;
}

void Client::setHttpResponse(const HttpResponse &response)
{
    _response = response;
}

HttpRequest &Client::getHttpRequest()
{
    return _request;
}

HttpResponse &Client::getHttpResponse()
{
    return _response;
}

int Client::getClientSocket()
{
    return _socket;
}

std::string Client::getIPaddress()
{
    char *cString = inet_ntoa(_address.sin_addr);
    return std::string(cString);
}

bool Client::isConnected() const
{
    return _socket >= 0;
}

 struct sockaddr_in& Client::getClientAddr()
 {
    return (_address);
 }