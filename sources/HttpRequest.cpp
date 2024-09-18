#include "HttpRequest.hpp"

// COPLIEN'S FORM
HttpRequest::HttpRequest() 
    : _method(""), _uri(""), _version("HTTP/1.1"), _headers(), _allowedMethods(initMethods()), _body(""), _queryParameters(""), _cookies(""), _isChunked(false) {}

HttpRequest::~HttpRequest() {}

HttpRequest::HttpRequest(const HttpRequest& src) 
    : _method(src._method), _uri(src._uri), _version(src._version), _headers(src._headers), _allowedMethods(src._allowedMethods), _body(src._body), _queryParameters(src._queryParameters), _cookies(src._cookies), _isChunked(src._isChunked) {}

HttpRequest& HttpRequest::operator=(const HttpRequest& src) {
    if (this != &src) {
        _method = src._method;
        _uri = src._uri;
        _version = src._version;
        _headers = src._headers;
        _body = src._body;
        _queryParameters = src._queryParameters;
        _allowedMethods = src._allowedMethods;
        // BONUS :
        // _cookies = src._cookies;
    }
    return *this;
}

// 1) REQUEST LINE
std::string HttpRequest::getMethod() const { return _method; }
std::string HttpRequest::getURI() const { return _uri; }
std::string HttpRequest::getHTTPVersion() const { return _version; }

// 2) HEADERS
std::map<std::string, std::string> HttpRequest::getHeaders() const { return _headers; }
std::string HttpRequest::getHeader(const std::string& name) const {
    std::map<std::string, std::string>::const_iterator it = _headers.find(name);
        if (it != _headers.end()) {
        return it->second;
    } else {
        return "";
    }
}

// TO DO LATER ALIGATOR
// accept
// accept-language
// accept-encoding
// connection
// cookie
// host
// user-agent

// 3) BODY
std::string HttpRequest::getBody() const { return _body; }
std::string HttpRequest::getQueryParameters() const { return _queryParameters; }
// BONUS :
// std::string HttpRequest::getCookies() const { return _cookies; }

 // OTHER
const std::set<std::string> HttpRequest::initMethods() {
    return {"GET", "POST", "DELETE"};
}

bool HttpRequest::isMethodAllowed(const std::string& method) const {
    return _allowedMethods.find(method) != _allowedMethods.end();
}

bool HttpRequest::isSupportedContentType(const std::string& contentType) const {
    static const std::set<std::string> supportedTypes = {
        "application/json", 
        "text/html", 
        "text/plain"
    };
    return supportedTypes.find(contentType) != supportedTypes.end();
}

// PRINT DATA
std::ostream& operator<<(std::ostream& os, const HttpRequest& req) {
    os << "Method: " << req.getMethod() << "\n";
    os << "URI: " << req.getURI() << "\n";
    os << "Version: " << req.getHTTPVersion() << "\n";
    os << "Headers: \n";
    for (const auto& header : req.getHeaders()) {
        os << "  " << header.first << ": " << header.second << "\n";
    }
    os << "Body: " << req.getBody() << "\n";
    os << "Query Parameters: " << req.getQueryParameters() << "\n";
    os << "Cookies: " << req.getCookies() << "\n";
    os << "Chunked: " << (req.isChunked() ? "Yes" : "No") << "\n";
    return os;
}
