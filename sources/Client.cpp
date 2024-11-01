#include "Client.hpp"

Client::Client(int fd, const struct sockaddr_in &address)
    : _socket(fd), _address(address), _request(), _response() {}

Client::~Client() {}

void Client::readRequest(const std::string &rawData)
{
    Logger &logger = Logger::getInstance("server.log");
    logger.log(">> rawData in readRequest to pass to HttpRequest(rawData) is " + rawData);
    _request = HttpRequest(rawData);
}

bool Client::checkFileExists(const std::string &filePath)
{
    struct stat buffer;
    return (stat(filePath.c_str(), &buffer) == 0);
}

void Client::processRequest(const ServerData &serverInfo, size_t maxSize)
{
    HttpResponse response;
    std::string uri;
    std::string method;

    Logger &logger = Logger::getInstance("server.log");

    uri = _request.getURI();
    method = _request.getMethod();
    logger.log("SIZE max: " + to_string(maxSize) + " current size " + to_string(_request.getBody().size()));

    logger.log("PATH: " + uri);

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

                logger.log("Redirection from " + uri + " to " + location.redirect.url + " with code " + std::to_string(location.redirect.code));
                return;
            }
        }

        if (_request.getBody().size() > maxSize)
        {
            response.generate413PayloadTooLarge(maxSize);
            _response = response;
            return;
        }

		if (_request.getBody().size() <= 0 && method == "POST")
		{
			response.generate405MethodNotAllowed("POST body not compliant");
            _response = response;
			return;
		}

        logger.log("Max size: " + to_string(maxSize) + " | Current size: " + to_string(_request.getBody().size()));

        const HttpConfig::Location *matchedLocation = findMatchingLocation(serverInfo, uri);

        if (matchedLocation == NULL)
        {
            response.generate404NotFound("The requested URL " + uri, serverInfo._root);
            logger.logError("404 Not Found for URI: " + uri);
        }
        else
        {
            const HttpConfig::Location &location = *matchedLocation;
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

    std::string uriPath = uri;
    size_t queryPos = uri.find('?');
    if (queryPos != std::string::npos)
    {
        uriPath = uri.substr(0, queryPos);
    }

    for (size_t i = 0; i < serverInfo._locations.size(); ++i)
    {
        const HttpConfig::Location &location = serverInfo._locations[i];
        if (uriPath == location.path)
        {
            logger.log("Exact match found: " + location.path);
            return &location;
        }
    }

    for (size_t i = 0; i < serverInfo._locations.size(); ++i)
    {
        const HttpConfig::Location &location = serverInfo._locations[i];
        if (location.path == "/")
            continue;

        if (uriPath.find(location.path) == 0)
        {
            if (location.path.length() > longestMatchLength)
            {
                matchedLocation = &location;
                longestMatchLength = location.path.length();
            }
        }
    }
    if (matchedLocation == NULL)
    {
        // Can return default location 404 or NULL
        logger.log("No matching location found for URI: " + uri);
        return NULL;
    }

    logger.log("Matched Location Path: " + matchedLocation->path);
    return matchedLocation;
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
                continue;
            }
            else
            {
                throw std::runtime_error("Error sending response: " + std::string(strerror(errno)));
            }
        }
        sent += n;
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
        return _request.getHTTPVersion() == "HTTP/1.1";
    }
    else
    {
        return (connectionHeader == "keep-alive");
    }
}