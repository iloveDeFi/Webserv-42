#include "Client.hpp"

Client::Client(int fd, const struct sockaddr_in &address)
    : _socket(fd), _address(address), _request(""), _response() {}

Client::~Client() {}

void Client::readRequest(const std::string &rawData)
{
    _request = HttpRequest(rawData);
}

void Client::processRequest(const _server &serverInfo)
{
    HttpResponse response;
    bool requestHandled = false;
    std::string uri = _request.getURI();
    // TO DO : URI value is good here
    // std::cout << "Requested URI: " << uri << std::endl;
    std::string method = _request.getMethod();

    // Instantiate Logger (logs will be written to "server.log") use cat
    Logger &logger = Logger::getInstance("server.log");

    for (size_t i = 0; i < serverInfo._locations.size(); ++i)
    {
        const HttpConfig::Location &location = serverInfo._locations[i];
        std::cout << "HERE!!!!! =>" << location.handler << std::endl;
    
        if (uri.find(location.path) == 0)
        {
            if (_request.getMethod() == "GET")
            {
                // std::cout << "------- Processing request - Method: " << method << ", URI: " << uri << std::endl; good here
                GetRequestHandler getHandler(location, serverInfo._root);
                getHandler.handle(_request, response);
                requestHandled = true;
            }
            else if (_request.getMethod() == "POST")
            {
                PostRequestHandler postHandler(location, serverInfo._root);
                postHandler.handle(_request, response);
                requestHandled = true;
            }
            else if (_request.getMethod() == "DELETE")
            {
                DeleteRequestHandler deleteHandler(location, serverInfo._root);
                deleteHandler.handle(_request, response);
                requestHandled = true;
            }
            else
            {
                UnknownRequestHandler unknownHandler(location, serverInfo._root);
                unknownHandler.handle(_request, response);
                requestHandled = true;
            }
            break;
        }
    }

    if (requestHandled)
    {
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