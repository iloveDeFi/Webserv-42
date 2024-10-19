#include "Client.hpp"

Client::Client(int fd, const struct sockaddr_in &address)
    : _socket(fd), _address(address), _request(""), _response() {}

Client::~Client() {}

void Client::readRequest(const std::string &rawData) {
    _request = HttpRequest(rawData);
}

void Client::processRequest(const _server &serverInfo) {
    HttpResponse response;
    bool requestHandled = false;
    std::string uri = _request.getURI();

    for (size_t i = 0; i < serverInfo._locations.size(); ++i) {
        const HttpConfig::Location &location = serverInfo._locations[i];

        if (uri.find(location.path) == 0) {
            if (_request.getMethod() == "GET") {
                GetRequestHandler getHandler(location);
                getHandler.handle(_request, response);
                requestHandled = true;
            } else if (_request.getMethod() == "POST") {
                PostRequestHandler postHandler(location);
                postHandler.handle(_request, response);
                requestHandled = true;
            } else if (_request.getMethod() == "DELETE") {
                DeleteRequestHandler deleteHandler(location);
                deleteHandler.handle(_request, response);
                requestHandled = true;
            } else {
                UnknownRequestHandler unknownHandler(location);
                unknownHandler.handle(_request, response);
                requestHandled = true;
            }
            break;
        }
    }

    if (requestHandled) {
        int statusCode = response.getStatusCode();
        if (statusCode >= 400) {
            std::cerr << "Erreur lors de la gestion de la requête : " << statusCode << std::endl;
        }
    } else {
        response.setStatusCode(404);
        response.setBody("404 Not Found");
        response.setHeader("Content-Type", "text/plain");
        response.ensureContentLength();
    }

    _response = response;
    sendResponse();
}

void Client::sendResponse() {
    std::string response = _response.toString();
    const char *data = response.c_str();
    size_t total = response.size();
    size_t sent = 0;
    ssize_t n;

    while (sent < total) {
        n = send(_socket, data + sent, total - sent, 0);
        if (n == -1)
            throw std::runtime_error("Error sending response.");
        sent += n;
    }
}

void Client::setHttpRequest(const HttpRequest &request) {
    _request = request;
}

void Client::setHttpResponse(const HttpResponse &response) {
    _response = response;
}

HttpRequest &Client::getHttpRequest() {
    return _request;
}

HttpResponse &Client::getHttpResponse() {
    return _response;
}

int Client::getClientSocket() {
    return _socket;
}

std::string Client::getIPaddress() {
    char *cString = inet_ntoa(_address.sin_addr);
    return std::string(cString);
}

bool Client::isConnected() const {
    return _socket >= 0;
}