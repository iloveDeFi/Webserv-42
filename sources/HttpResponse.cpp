#include "HttpResponse.hpp"
#include <sstream>

HttpResponse::HttpResponse() : AHttpMessage(), _statusCode(200), _statusMessage("OK") {
    setVersion("HTTP/1.1");
}

HttpResponse::~HttpResponse() {}

HttpResponse::HttpResponse(const HttpResponse& src)
    : AHttpMessage(src), _statusCode(src._statusCode), _statusMessage(src._statusMessage) {}

HttpResponse& HttpResponse::operator=(const HttpResponse& src) {
    if (this != &src) {
        AHttpMessage::operator=(src);
        _statusCode = src._statusCode;
        _statusMessage = src._statusMessage;
    }
    return *this;
}

void HttpResponse::setHeader(const std::string& name, const std::string& value) {
    _headers[name] = value;
}

void HttpResponse::setStatusCode(int statusCode) {
    _statusCode = statusCode;
    _statusMessage = getStatusMessage(_statusCode);
}

std::string HttpResponse::getStatusMessage(int statusCode) const {
    switch (statusCode) {
        case 200: return "OK";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        // Add more status codes as needed
        default: return "Unknown Status";
    }
}

void HttpResponse::ensureContentLength() {
    if (_headers.find("Content-Length") == _headers.end()) {
        std::stringstream ss;
        ss << getBody().length();
        setHeader("Content-Length", ss.str());
    }
}

std::string HttpResponse::generate404Error(const std::string& uri) {
    setStatusCode(404);
    std::string errorPage = "<html><body><h1>404 Not Found</h1>";
    errorPage += "<p>The requested resource " + uri + " was not found on this server.</p></body></html>";
    
    setBody(errorPage);
    setHeader("Content-Type", "text/html");
    ensureContentLength();
    return responseToString();
}

std::string HttpResponse::generateRedirection(const std::string& newUri) {
    setStatusCode(302);
    setHeader("Location", newUri);
    ensureContentLength();
    return responseToString();
}

std::string HttpResponse::responseToString() const {
    std::stringstream response;
    response << getVersion() << " " << _statusCode << " " << _statusMessage << "\r\n";
    
    std::map<std::string, std::string>::const_iterator it;
    for (it = _headers.begin(); it != _headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }

    response << "\r\n" << getBody();
    return response.str();
}

void HttpResponse::setBody(const std::string& body) {
    AHttpMessage::setBody(body);
    ensureContentLength();
}

int HttpResponse::getStatusCode() const {
    return _statusCode;
}

std::string HttpResponse::getStatusMessage() const {
    return _statusMessage;
}

std::ostream& operator<<(std::ostream& os, const HttpResponse& res) {
    os << "HTTP Version: " << res.getVersion() << "\n";
    os << "Status Code: " << res.getStatusCode() << "\n";
    os << "Status Message: " << res.getStatusMessage() << "\n";
    os << "Headers: \n";
    std::map<std::string, std::string> headers = res.getHeaders();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        os << "  " << it->first << ": " << it->second << "\n";
    }
    os << "Body: " << res.getBody() << "\n";
    return os;
}