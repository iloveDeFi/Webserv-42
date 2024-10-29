#include "Client.hpp"

Client::Client(int fd, const struct sockaddr_in &address)
    : _socket(fd), _address(address), _request(), _response() {}

Client::~Client() {}

void Client::readRequest(const std::string &rawData)
{
    //_request.parse(rawData);
    // Logger &logger = Logger::getInstance("server.log");
    // logger.log(">> rawData in readRequest to pass to HttpRequest(rawData) is " + rawData);
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

void Client::processRequest(const ServerData &serverInfo, size_t maxSize)
{
    HttpResponse response;
    std::string uri;
    std::string method;

    Logger &logger = Logger::getInstance("server.log");

    uri = _request.getURI();
    method = _request.getMethod();
    logger.logError("SIZE max" + to_string(maxSize) + " current size " + to_string(_request.getBody().size()));

    logger.log("PATH: " + uri);

    try
    {
        for (size_t i = 0; i < serverInfo._locations.size(); ++i)
        {
            const HttpConfig::Location &location = serverInfo._locations[i];

            if (uri == location.path && !location.redirect.url.empty())
            {
                _response = HttpResponse();

                if (location.redirect.code == 301)
                {
                    _response.generate301MovedPermanently(location.redirect.url);
                }
                else
                {
                    _response.setStatusCode(location.redirect.code);
                    _response.setHeader("Location", location.redirect.url);
                    _response.setBody("Redirecting to " + location.redirect.url);
                    _response.setHeader("Content-Length", std::to_string(_response.getBody().size()));
                }

                // -------------------------------------

                logger.log("Redirection from " + uri + " to " + location.redirect.url + " with code " + std::to_string(location.redirect.code));
                return;
            }
        }

        if (_request.getBody().size() > maxSize)
        {
            response.generate413PayloadTooLarge(maxSize);
            _response = response;
            return; // Terminer le traitement si la taille est excessive
        }
        // Vérification de la taille maximale autorisée du body
        logger.logError("Max size: " + to_string(maxSize) + " | Current size: " + to_string(_request.getBody().size()));

        const HttpConfig::Location *exactMatch = nullptr;

        // Recherche d'une correspondance exacte pour l'URI
        for (size_t i = 0; i < serverInfo._locations.size(); ++i)
        {
            const HttpConfig::Location &location = serverInfo._locations[i];

            // Vérifie si l'URI correspond exactement au chemin
            if (uri == location.path)
            {
                exactMatch = &location;
                break; // Sortir dès qu'on trouve une correspondance exacte
            }
        }

        // Si aucune correspondance exacte n'est trouvée, générer une réponse 404
        if (exactMatch == nullptr)
        {
            response.generate404NotFound("The requested URL " + uri, serverInfo._root);
            logger.logError("404 Not Found for URI: " + uri);
        }
        else
        {
            // Gestion de la requête en fonction de la méthode
            const HttpConfig::Location &location = *exactMatch;
            std::cout << "Matched Location Path: " << location.path << ", Handler: " << location.handler << std::endl;

            if (location.iscgi)
            {
                CgiRequestHandler cgiHandler(location, serverInfo);
                cgiHandler.handle(_request, response, serverInfo);
            }
            else if (method == "GET")
            {
                GetRequestHandler getHandler(location, serverInfo);
                getHandler.handle(_request, response, serverInfo);
            }
            else if (method == "POST")
            {
                PostRequestHandler postHandler(location, serverInfo);
                postHandler.handle(_request, response, serverInfo);
            }
            else if (method == "DELETE")
            {
                DeleteRequestHandler deleteHandler(location, serverInfo);
                deleteHandler.handle(_request, response, serverInfo);
            }
            else if (method == "OPTIONS")
            {
                OptionsRequestHandler optionsHandler(location, serverInfo);
                optionsHandler.handle(_request, response, serverInfo);
            }
            else
            {
                logger.log("UNKNOWN method detected.");
                UnknownRequestHandler unknownHandler(location, serverInfo);
                unknownHandler.handle(_request, response, serverInfo);
            }
        }

        // Log du code de statut de la réponse
        int statusCode = response.getStatusCode();
        logger.logRequest(method, uri, statusCode);

        // Si le code de statut est une erreur, log supplémentaire
        if (statusCode >= 400)
        {
            logger.logError("Request resulted in error: " + to_string(statusCode));
        }

        // Assigner la réponse à l'attribut de réponse du client
        _response = response;
    }
    catch (const std::exception &e)
    {
        // En cas d'exception, générer une réponse d'erreur 400
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

bool Client::isKeepAlive() const
{
    std::string connectionHeader = _request.getHeader("Connection");
    if (connectionHeader.empty())
    {
        // HTTP/1.1 default is keep-alive
        return _request.getHTTPVersion() == "HTTP/1.1";
    }
    else
    {
        return (connectionHeader == "keep-alive");
    }
}