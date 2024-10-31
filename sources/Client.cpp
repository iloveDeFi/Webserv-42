#include "Client.hpp"

Client::Client(int fd, const struct sockaddr_in &address)
    : _socket(fd), _address(address), _request(), _response() {}

Client::~Client() {}

void Client::readRequest(const std::string &rawData)
{
    //_request.parse(rawData);
    Logger &logger = Logger::getInstance("server.log");
    logger.log(">> rawData in readRequest to pass to HttpRequest(rawData) is " + rawData);
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

std::string Client::join(const std::vector<std::string>& strings, const std::string& delimiter) {
    std::ostringstream result; // Use ostringstream for constructing the string

    for (size_t i = 0; i < strings.size(); ++i) {
        result << strings[i]; // Add the current string
        if (i < strings.size() - 1) {
            result << delimiter; // Add the delimiter if not the last element
        }
    }

    return result.str(); // Return the constructed string
}

void Client::processRequest(const ServerData &serverInfo, size_t maxSize)
{
    HttpResponse response;
    std::string uri;
    std::string method;

    Logger &logger = Logger::getInstance("server.log");

    uri = _request.getURI();
    method = _request.getMethod();

    logger.log("SIZE max" + to_string(maxSize) + " current size " + to_string(_request.getBody().size()));
    logger.log("PATH: " + uri);

    

    // TO DO TEST : Redirection conditionnelle pour gérer "/directory" et "/directory/"
    if (uri == "/directory" && method == "GET") {
        HttpResponse redirectResponse;
        redirectResponse.setStatusCode(301); // Code de redirection 301
        redirectResponse.setHeader("Location", "/directory/"); // URL redirigée
        redirectResponse.setBody("Redirecting to /directory/"); // Corps de la réponse (optionnel)
        redirectResponse.setHeader("Content-Length", std::to_string(redirectResponse.getBody().size()));

        logger.log("Redirection from " + uri + " to /directory/ with status 301");
        _response = redirectResponse;
        return;
    }


    try
    {
        for (size_t i = 0; i < serverInfo._locations.size(); ++i)
        {
            const HttpConfig::Location &location = serverInfo._locations[i];

			if (location.redirect.code == 405)
			{
				_response.setStatusCode(location.redirect.code);
			}

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

        // Check if body size exceeds max size
        if (_request.getBody().size() > maxSize)
        {
            response.generate413PayloadTooLarge(maxSize);
            _response = response;
            return; // Terminate processing if size is too large
        }

		if (_request.getBody().size() <= 0 && method == "POST")
		{
			response.generate405MethodNotAllowed("POST body not compliant");
            _response = response;
			return;
		}

        // Log body size
        logger.log("Max size: " + to_string(maxSize) + " | Current size: " + to_string(_request.getBody().size()));

        // Find the best matching location
        // const HttpConfig::Location *matchedLocation = findMatchingLocation(serverInfo, uri);

        // TO DO TEST instead of : const HttpConfig::Location *matchedLocation = findMatchingLocation(serverInfo, uri);
        const HttpConfig::Location *matchedLocation = NULL; // Utiliser NULL au lieu de nullptr en C++98
        for (size_t i = 0; i < serverInfo._locations.size(); ++i) {
            const HttpConfig::Location &location = serverInfo._locations[i]; // Accéder à l'élément par son index
            if (uri == location.path) {
                matchedLocation = &location; // Faire correspondre l'emplacement
                break; 
            }
        }

        if (matchedLocation == NULL)
        {
            response.generate404NotFound("The requested URL " + uri, serverInfo._root);
            logger.logError("404 Not Found for URI: " + uri);
            _response = response;
            return;
        }
            // Gestion de la requête en fonction de la méthode HTTP
            const HttpConfig::Location &location = *matchedLocation;
            std::cout << "Matched Location Path: " << location.path << ", Handler: " << location.handler << std::endl;

            // TO DO : add 405 method handler first for security and no bug?
             // Vérifier si la méthode est autorisée
        if (std::find(matchedLocation->methods.begin(), matchedLocation->methods.end(), method) == matchedLocation->methods.end()) {
            response.generate405MethodNotAllowed("Method " + method + " not allowed for " + uri);
            logger.logError("405 Method Not Allowed for URI: " + uri);
            logger.log(">>>>>Allowed methods for " + uri + ": " + join(matchedLocation->methods, ", "));
            _response = response;
            return;
        }

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

        // Log the response status code
        int statusCode = response.getStatusCode();
        logger.logRequest(method, uri, statusCode);

        // Additional logging if status code indicates an error
        if (statusCode >= 400)
        {
            logger.logError("Request resulted in error: " + to_string(statusCode));
        }

        // Assign the response to the client's response attribute
        _response = response;
    }
    catch (const std::exception &e)
    {
        // In case of exception, generate a 400 Bad Request response
        response.setStatusCode(400);
        response.setBody("400 Bad Request: " + std::string(e.what()));
        response.setHeader("Content-Type", "text/plain");
        response.ensureContentLength();
        logger.logError("Exception in processing request: " + std::string(e.what()));
        _response = response;
    }
}

bool Client::handleRedirect(const ServerData &serverInfo, const std::string &method, HttpResponse &response)
{
    Logger &logger = Logger::getInstance("server.log");

    for (size_t i = 0; i < serverInfo._locations.size(); ++i)
    {
        const HttpConfig::Location &location = serverInfo._locations[i];
        if (location.redirect.code == 301 && method == "GET")
        {
            response.generate301MovedPermanently(location.redirect.url);
            logger.log("Redirecting to: " + location.redirect.url);
            logger.log("Code is: " + to_string(location.redirect.code));
            return true;
        }
    }
    return false;
}

const HttpConfig::Location* Client::findMatchingLocation(const ServerData &serverInfo, const std::string &uri)
{
    Logger &logger = Logger::getInstance("server.log");
    const HttpConfig::Location *matchedLocation = NULL;
    size_t longestMatchLength = 0;

    // Remove query parameters for matching
    std::string uriPath = uri;
    size_t queryPos = uri.find('?');
    if (queryPos != std::string::npos)
    {
        uriPath = uri.substr(0, queryPos);
    }

    // First, attempt exact matching
    for (size_t i = 0; i < serverInfo._locations.size(); ++i)
    {
        const HttpConfig::Location &location = serverInfo._locations[i];
        if (uriPath == location.path)
        {
            logger.log("Exact match found: " + location.path);
            return &location;
        }
    }

    // Then, attempt longest prefix matching (excluding '/')
    for (size_t i = 0; i < serverInfo._locations.size(); ++i)
    {
        const HttpConfig::Location &location = serverInfo._locations[i];
        if (location.path == "/")
            continue; // Skip the default location for now

        if (uriPath.find(location.path) == 0)
        {
            if (location.path.length() > longestMatchLength)
            {
                matchedLocation = &location;
                longestMatchLength = location.path.length();
            }
        }
    }

    // If still no match, decide whether to use default location or return NULL
    if (matchedLocation == NULL)
    {
        // Optionally, you can choose to not use the default location to force a 404
        logger.log("No matching location found for URI: " + uri);
        return NULL;
    }

    logger.log("Matched Location Path: " + matchedLocation->path);
    return matchedLocation;
}


// void Client::handleRequest(const HttpRequest &request, const HttpConfig::Location &location, const ServerData &serverInfo, HttpResponse &response)
// {
//     Logger &logger = Logger::getInstance("server.log");
//     std::string method = request.getMethod();

//     std::cout << "Matched Location Path: " << location.path << ", Handler: " << location.handler << std::endl;

//     if (location.iscgi)
//     {
//         CgiRequestHandler cgiHandler(location, serverInfo);
//         cgiHandler.handle(request, response);
//     }
//     else if (method == "GET")
//     {
//         GetRequestHandler getHandler(location, serverInfo);
//         getHandler.handle(request, response);
//     }
//     else if (method == "POST")
//     {
//         PostRequestHandler postHandler(location, serverInfo);
//         postHandler.handle(request, response);
//     }
//     else if (method == "DELETE")
//     {
//         DeleteRequestHandler deleteHandler(location, serverInfo);
//         deleteHandler.handle(request, response);
//     }
//     else if (method == "OPTIONS")
//     {
//         OptionsRequestHandler optionsHandler(location, serverInfo);
//         optionsHandler.handle(request, response);
//     }
//     else
//     {
//         logger.log("UNKNOWN method detected.");
//         UnknownRequestHandler unknownHandler(location, serverInfo);
//         unknownHandler.handle(request, response);
//     }
// }



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