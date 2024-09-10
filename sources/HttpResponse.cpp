#include "HttpResponse.hpp"

HttpResponse::HttpResponse() : _statusCode(200), _version("HTTP/1.1") {}

HttpResponse::~HttpResponse() {}

HttpResponse::HttpResponse(const HttpResponse& src)
    : _statusCode(src._statusCode), _version(src._version), _headers(src._headers), _body(src._body) {}

HttpResponse& HttpResponse::operator=(const HttpResponse& src) {
    if (this != &src) {
        _statusCode = src._statusCode;
        _version = src._version;
        _headers = src._headers;
        _body = src._body;
    }
    return *this;
}

void HttpResponse::setHeader(const std::string& name, const std::string& value) {
    _headers[name] = value;
}

std::string HttpResponse::setStatusMessage(HttpRequest& req) {
    return getStatusMessage(req._statusCode);
}

void HttpResponse::ensureContentLength() {
    // define header content length if dosent exist
    if (_headers.find("Content-Length") == _headers.end()) {
        _headers["Content-Length"] = std::string("0");
    }
}

std::string HttpResponse::generate404Error(const std::string& uri) {
    std::string errorPage = "<html><body><h1>404 Not Found</h1>";
    errorPage += "<p>The requested resource " + uri + " was not found on this server.</p></body></html>";
    
    return "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n" + errorPage;
}

std::string HttpResponse::generateRedirection(const std::string& newUri) {
    return "HTTP/1.1 302 Found\r\nLocation: " + newUri + "\r\n\r\n";
}

std::string HttpResponse::responseToString() const {
    std::string response = _version + " " + std::to_string(_statusCode) + " " + getStatusMessage() + "\r\n";
    
    std::map<std::string, std::string>::const_iterator it;
    for (it = _headers.begin(); it != _headers.end(); ++it) {
        response += it->first + ": " + it->second + "\r\n";
    }

    response += "\r\n" + _body;
    return response;
}