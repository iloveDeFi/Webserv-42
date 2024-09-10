#include "HttpRequest.hpp"

HttpRequest::HttpRequest() : _method(""), _uri(""), _version("HTTP/1.1"), _allowedMethods(initMethods()) {}

HttpRequest::~HttpRequest() {}

HttpRequest::HttpRequest(const HttpRequest& src) : _method(src._method), _uri(src._uri), _version(src._version), _headersAsString(src._headers), _body(src._body) {}

HttpRequest& HttpRequest::operator=(const HttpRequest& src) {
    if (this != &src) {
        this->_method = src._method;
        this->_uri = src._uri;
        this->_version = src._version;
        this->_headersAsString = src._headersAsString;
        this->_body = src._body;
        this->_is_chunked = src._is_chunked;
    }
    return *this;
}

std::string HttpRequest::getMethod() const { return _method; }
std::string HttpRequest::getURI() const { return _uri; }
std::string HttpRequest::getHTTPVersion() const { return _version; }

std::map<std::string, std::string> HttpRequest::getHeaders() const { return _headersAsString; }
std::string HttpRequest::getHeader(const std::string& name) const {
    std::map<std::string, std::string>::const_iterator it = _headersAsString.find(name);
        if (it != _headersAsString.end()) {
        return it->second;
    } else {
        return "";
    }
}

std::string HttpRequest::getBody() const { return _body; }
std::string HttpRequest::getQueryParameters() const { return _queryParameters; }
std::string HttpRequest::getCookies() const { return _cookies; }
bool HttpRequest::getIsChunked() const { return _is_chunked; }