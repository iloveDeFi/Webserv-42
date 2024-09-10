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

std::string HttpResponse::getStatusMessage() const {
    switch (_statusCode) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        default: return "Unknown Status";
    }
}

// ----- SETTERS ----- :
// 1) RESPONSE LINE
std::string HttpResponse::setMethod(const std::string& method) { _method = method; }
std::string HttpResponse::setURI(const std::string& uri) { _uri = uri; }
std::string HttpResponse::setHTTPVersion(const std::string& version) { _version = version; }
// 2) HEADERS
void HttpResponse::setHeaders(const std::map<std::string, std::string>& headers) { _headers = headers; }
// 3) BODY
void HttpResponse::setBody(const std::string& body) { _body = body; }
void HttpResponse::setIsChunked(bool is_chunked) { _is_chunked = is_chunked; }
void HttpResponse::ensureContentLength() {
    // define header content length if dosent exist
    if (_headers.find("Content-Length") == _headers.end()) {
        _headers["Content-Length"] = std::string("0");
    }
}

std::string HttpResponse::toString() const {
    std::string response = _version + " " + std::to_string(_statusCode) + " " + getStatusMessage() + "\r\n";
    
    std::map<std::string, std::string>::const_iterator it;
    for (it = _headers.begin(); it != _headers.end(); ++it) {
        response += it->first + ": " + it->second + "\r\n";
    }
    response += "\r\n" + _body;
    return response;
}

std::string HttpResponse::generate404Error(const std::string& uri) {
    std::string errorPage = "<html><body><h1>404 Not Found</h1>";
    errorPage += "<p>The requested resource " + uri + " was not found on this server.</p></body></html>";
    
    return "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n" + errorPage;
}

std::string HttpResponse::generateRedirection(const std::string& newUri) {
    return "HTTP/1.1 302 Found\r\nLocation: " + newUri + "\r\n\r\n";
}
