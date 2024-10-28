#include "Client.hpp"

Client::Client(int fd, const struct sockaddr_in &address)
    : _socket(fd), _address(address), _request(), _response() {}

Client::~Client() {}

void Client::readRequest(const std::string &rawData)
{
    //_request.parse(rawData);
    //Logger &logger = Logger::getInstance("server.log");
    //logger.log(">> rawData in readRequest to pass to HttpRequest(rawData) is " + rawData);
    _request = HttpRequest(rawData);
    /*     std::cout << "Method: " << _request.getMethod() << std::endl;
        std::cout << "URI: " << _request.getURI() << std::endl;
        std::cout << "Version: " << _request.getHTTPVersion() << std::endl; */
}

bool Client::checkFileExists(const std::string &filePath)
{
    struct stat buffer;
    return (stat(filePath.c_str(), &buffer) == 0); // Renvoie true si le fichier existe
}

void Client::processRequest(const _server &serverInfo)
{
    HttpResponse response;
    std::string uri;
    std::string method;

    Logger &logger = Logger::getInstance("server.log");

    try
    {
        uri = _request.getURI();
        method = _request.getMethod();

        const HttpConfig::Location *exactMatch = NULL;

        // Chercher la location avec une correspondance exacte
        for (size_t i = 0; i < serverInfo._locations.size(); ++i)
        {
            const HttpConfig::Location &location = serverInfo._locations[i];

            // Vérifiez si l'URI correspond exactement à la path
            if (uri == location.path)
            {
                exactMatch = &location;
                break; // Sortir dès qu'on trouve une correspondance exacte
            }
        }

        // Si aucune correspondance n'est trouvée, générer une réponse 404
        if (exactMatch == NULL)
        {
            response.generate404NotFound("The requested URL " + uri + " was not found on this server.");
            logger.logError("404 Not Found for URI: " + uri);
        }
        else
        {
            // Gestion des requêtes selon le type de méthode
            const HttpConfig::Location &location = *exactMatch;
            std::cout << "Matched Location Path: " << location.path << ", Handler: " << location.handler << std::endl;

            if (location.iscgi)
            {
                CgiRequestHandler cgiHandler(location, serverInfo._root);
                cgiHandler.handle(_request, response);
            }
            else if (method == "GET")
            {
                GetRequestHandler getHandler(location, serverInfo._root);
                getHandler.handle(_request, response);
            }
            else if (method == "POST")
            {
                PostRequestHandler postHandler(location, serverInfo._root);
                postHandler.handle(_request, response);
            }
            else if (method == "DELETE")
            {
                DeleteRequestHandler deleteHandler(location, serverInfo._root);
                deleteHandler.handle(_request, response);
            }
            else if (method == "OPTIONS")
            {
                OptionsRequestHandler optionsHandler(location, serverInfo._root);
                optionsHandler.handle(_request, response);
            }
            else
            {
                logger.log("UNKNOWN method detected.");
                UnknownRequestHandler unknownHandler(location, serverInfo._root);
                unknownHandler.handle(_request, response);
            }
        }

        int statusCode = response.getStatusCode();
        logger.logRequest(method, uri, statusCode);
        if (statusCode >= 400)
        {
            logger.logError("Request resulted in error: " + to_string(statusCode));
        }

        _response = response;
    }
    catch (const std::exception &e)
    {
        response.setStatusCode(400);
        response.setBody("400 Bad Request: " + std::string(e.what()));
        response.setHeader("Content-Type", "text/plain");
        response.ensureContentLength();
        logger.logError("Exception in processing request: " + std::string(e.what()));
        _response = response;
    }
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

struct sockaddr_in &Client::getClientAddr()
{
    return (_address);
}