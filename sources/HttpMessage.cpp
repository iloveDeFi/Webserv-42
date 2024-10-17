#include "../includes/HttpMessage.hpp"

// Constructors and Destructor
HttpMessage::HttpMessage() {
    _version = "HTTP/1.1";
    _headers = std::map<std::string, std::string>();
    _body = "";
}

HttpMessage::~HttpMessage() {}

// Getters
std::string HttpMessage::getHTTPVersion() const {
    return _version;
}

std::map<std::string, std::string> HttpMessage::getHeaders() const {
    return _headers;
}

std::string HttpMessage::getHeader(const std::string& name) const {
    std::map<std::string, std::string>::const_iterator it = _headers.find(name);
    if (it != _headers.end()) {
        return it->second;
    } else {
        return "";
    }
}

std::string HttpMessage::getBody() const {
    return _body;
}

// Setters
void HttpMessage::setHTTPVersion(const std::string& version) {
    _version = version;
}

void HttpMessage::setHeaders(const std::map<std::string, std::string>& headers) {
    _headers = headers;
}

void HttpMessage::setHeader(const std::string& name, const std::string& value) {
    _headers[name] = value;
}

void HttpMessage::setBody(const std::string& body) {
    _body = body;
}

// Utility Functions
std::string HttpMessage::safe_substr(const std::string& str, size_t start, size_t length) const {
    if (start >= str.size()) throw std::out_of_range("Substring extraction failed: start index out of bounds.");
    if (start + length > str.size()) throw std::out_of_range("Substring extraction failed: length out of bounds.");
    return str.substr(start, length);
}

std::string HttpMessage::trim(const std::string& str) const {
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return safe_substr(str, first, last - first + 1);
}
