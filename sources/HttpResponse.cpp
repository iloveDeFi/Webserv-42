#include "HttpResponse.hpp"

HttpResponse::HttpResponse() 
    : _statusCode(200), _version("HTTP/1.1"), _headers(), _body(""), _isChunked(false) {}

HttpResponse::~HttpResponse() {}

HttpResponse::HttpResponse(const HttpResponse& src)
    : _statusCode(src._statusCode), _version(src._version), _headers(src._headers), _body(src._body), _isChunked(src._isChunked) {}

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
// 1) response line
void HttpResponse::setHTTPVersion(const std::string& version) { _version = version; }
void HttpResponse::setStatusCode(int statusCode) { _statusCode = statusCode; }
void HttpResponse::setReasonMessage(const std::string& reasonMessage) { _reasonMessage = reasonMessage; }
// 2) headers
void HttpResponse::setHeader(const std::string& name, const std::string& value) { _headers[name] = value; }
void HttpResponse::setHeaders(const std::map<std::string, std::string>& headers) { _headers = headers; }
// 3) body
void HttpResponse::setBody(const std::string& body) { _body = body; }
void HttpResponse::setIsChunked(bool isChunked) { _isChunked = isChunked; }

void HttpResponse::ensureContentLength() {
    if (_headers.find("Content-Length") == _headers.end()) {
        _headers["Content-Length"] = std::to_string(_body.size());
    }
}

std::string HttpResponse::toString() const {
    std::string response = _version + " " + std::to_string(_statusCode) + " " + _reasonMessage + "\r\n";
    
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
        response += it->first + ": " + it->second + "\r\n";
    }
    
    response += "\r\n" + _body;
    return response;
}

std::string HttpResponse::generate404Error(const std::string& uri) {
    std::string errorPage = "<html><body><h1>404 Not Found</h1>";
    errorPage += "<p>The requested resource " + uri + " was not found on this server.</p></body></html>";
    
    return "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(errorPage.size()) + "\r\n\r\n" + errorPage;
}

std::string HttpResponse::generateRedirection(const std::string& newUri) {
    return "HTTP/1.1 302 Found\r\nLocation: " + newUri + "\r\n\r\n";
}

std::ostream& HttpResponse::print(std::ostream& os) const {
    os << "--- RESPONSE LINE INFOS: ---" << std::endl;
    os << "Version is: " <<  _version << " Status code is: " << _statusCode << " Reason Message is: " << _reasonMessage << "\n\n";

    os << "--- HEADER INFOS: ---" << std::endl;
    os << "Headers name: " << _name << std::endl;
    os << "Headers value: " << _value << std::endl;
    os << "Headers are: " << _headers << std::endl;

    os << "--- BODY INFOS: ---" << std::endl;
    os << "Content-Length: " << _body.size() << "\n\n";
    os << "Body is: " << _body << "\n";
    os << "Body chunked: "<< _isChunked << "\n";
    return os;
}