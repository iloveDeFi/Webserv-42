#include "HttpResponse.hpp"

// Constructor
HttpResponse::HttpResponse() 
    : _statusCode(200), _version("HTTP/1.1"), _headers(), _body(""), _isChunked(false) {}

// Destructor
HttpResponse::~HttpResponse() {}

// Copy Constructor
HttpResponse::HttpResponse(const HttpResponse& src)
    : _statusCode(src._statusCode), _version(src._version), _headers(src._headers), _body(src._body), _isChunked(src._isChunked) {}

// Assignment Operator
HttpResponse& HttpResponse::operator=(const HttpResponse& src) {
    if (this != &src) {
        _statusCode = src._statusCode;
        _version = src._version;
        _headers = src._headers;
        _body = src._body;
        _isChunked = src._isChunked;
    }
    return *this;
}

// Setters
void HttpResponse::setStatusCode(int statusCode) { _statusCode = statusCode; }
void HttpResponse::setHTTPVersion(const std::string& version) { _version = version; }
void HttpResponse::setHeaders(const std::map<std::string, std::string>& headers) { _headers = headers; }
void HttpResponse::setHeader(const std::string& name, const std::string& value) { _headers[name] = value; }
void HttpResponse::setBody(const std::string& body) { _body = body; }
void HttpResponse::setIsChunked(bool isChunked) { _isChunked = isChunked; }

// Ensure Content-Length header is set
void HttpResponse::ensureContentLength() {
    if (_headers.find("Content-Length") == _headers.end()) {
        _headers["Content-Length"] = std::to_string(_body.size());
    }
}

// Convert response to string
std::string HttpResponse::toString() const {
    std::string response = _version + " " + std::to_string(_statusCode) + " " + getStatusMessage() + "\r\n";
    
    for (const auto& header : _headers) {
        response += header.first + ": " + header.second + "\r\n";
    }
    response += "\r\n" + _body;
    return response;
}

// Generate 404 error page
std::string HttpResponse::generate404Error(const std::string& uri) {
    std::string errorPage = "<html><body><h1>404 Not Found</h1>";
    errorPage += "<p>The requested resource " + uri + " was not found on this server.</p></body></html>";
    
    return "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(errorPage.size()) + "\r\n\r\n" + errorPage;
}

// Generate redirection response
std::string HttpResponse::generateRedirection(const std::string& newUri) {
    return "HTTP/1.1 302 Found\r\nLocation: " + newUri + "\r\n\r\n";
}
