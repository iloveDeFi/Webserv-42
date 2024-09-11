#include "HttpRequest.hpp"

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
        _allowedMethods = src._allowedMethods;
        _body = src._body;
        _queryParameters = src._queryParameters;
        _cookies = src._cookies;
        _isChunked = src._isChunked;
    }
    return *this;
}

// REQUEST LINE
std::string HttpRequest::getMethod() const { return _method; }
std::string HttpRequest::getURI() const { return _uri; }
std::string HttpRequest::getHTTPVersion() const { return _version; }

// HEADER
std::map<std::string, std::string> HttpRequest::getHeaders() const { return _headers; }
std::string HttpRequest::getHeader(const std::string& name) const {
    std::map<std::string, std::string>::const_iterator it = _headers.find(name);
        if (it != _headers.end()) {
        return it->second;
    } else {
        return "";
    }
}

// BODY
std::string HttpRequest::getBody() const { return _body; }
std::string HttpRequest::getQueryParameters() const { return _queryParameters; }
std::string HttpRequest::getCookies() const { return _cookies; }
bool HttpRequest::isChunked() const { return _isChunked; }

 // OTHER
const std::set<std::string> HttpRequest::initMethods() {
    return {"GET", "POST", "DELETE"};
}

bool HttpRequest::isMethodAllowed(const std::string& method) const {
    return _allowedMethods.find(method) != _allowedMethods.end();
}

void HttpRequest::requestController(HttpResponse& response, std::map<std::string, std::string>& resourceDatabase) {
    if (!isMethodAllowed(getMethod())) {
        response.setStatusCode(405);
        response.addHeader("Allow", "GET, POST, DELETE");
        response.setBody("405 Method Not Allowed");
    } else {
        if (getMethod() == "GET") {
            GetRequestHandler().handle(*this, response);
        } else if (getMethod() == "POST") {
            PostRequestHandler().handle(*this, response);
        } else if (getMethod() == "DELETE") {
            DeleteRequestHandler().handle(*this, response);
        }
    }
}

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
