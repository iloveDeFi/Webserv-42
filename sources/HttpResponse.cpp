#include "HttpResponse.hpp"

HttpResponse::HttpResponse() 
    : _httpVersion("HTTP/1.1"), _statusCode(200), _headers(), _body(""), _isChunked(false) {}

HttpResponse::~HttpResponse() {}

HttpResponse::HttpResponse(const HttpResponse& src)
    : _httpVersion(src._httpVersion), _statusCode(src._statusCode), _headers(src._headers), _body(src._body), _isChunked(src._isChunked) {}

HttpResponse& HttpResponse::operator=(const HttpResponse& src) {
    if (this != &src) {
        _httpVersion = src._httpVersion;
        _statusCode = src._statusCode;
        _reasonMessage = src._reasonMessage;
        _headers = src._headers;
        _body = src._body;
        _isChunked = src._isChunked;
    }
    return *this;
}

// Setters
// 1) RESPONSE LINE
void HttpResponse::setHTTPVersion(const std::string& httpVersion) { _httpVersion = httpVersion; }
void HttpResponse::setStatusCode(int statusCode) { _statusCode = statusCode; }
void HttpResponse::setReasonMessage(const std::string& reasonMessage) { _reasonMessage = reasonMessage; }

// 2) HEADERS
void HttpResponse::setHeader(const std::string& name, const std::string& value) { _headers[name] = value; }
void HttpResponse::setHeaders(const std::map<std::string, std::string>& headers) { _headers = headers; }

// 3) BODY
void HttpResponse::setBody(const std::string& body) { _body = body; }
void HttpResponse::setIsChunked(bool isChunked) { _isChunked = isChunked; }

// OTHER
void HttpResponse::ensureContentLength() {
    if (_headers.find("Content-Length") == _headers.end()) {
        _headers["Content-Length"] = std::to_string(_body.size());
    }
}

std::string HttpResponse::toString() const {
    std::string response = _httpVersion + " " + std::to_string(_statusCode) + " " + _reasonMessage + "\r\n";
    
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
        response += it->first + ": " + it->second + "\r\n";
    }
    
    response += "\r\n" + _body;
    return response;
}

// SPECIAL RESPONSES 
std::string HttpResponse::generate404Error(const std::string& uri) {
    std::string errorPage = "<html><body><h1>404 Not Found</h1>";
    errorPage += "<p>The requested resource " + uri + " was not found on this server.</p></body></html>";
    
    return "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(errorPage.size()) + "\r\n\r\n" + errorPage;
}

std::string HttpResponse::generateRedirection(const std::string& newUri) {
    return "HTTP/1.1 302 Found\r\nLocation: " + newUri + "\r\n\r\n";
}

// PRINT DATA
std::ostream& HttpResponse::print(std::ostream& os) const {
    os << "--- RESPONSE LINE INFOS: ---" << std::endl;
    os << "httpVersion is: " <<  _httpVersion << " Status code is: " << _statusCode << " Reason Message is: " << _reasonMessage << "\n\n";

    os << "--- HEADER INFOS: ---" << std::endl;
    os << "Headers are: " << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
        os << it->first << ": " << it->second << std::endl;
    }

    os << "--- BODY INFOS: ---" << std::endl;
    os << "Content-Length: " << _body.size() << "\n\n";
    os << "Body is: " << _body << "\n";
    os << "Body chunked: " << _isChunked << "\n";
    return os;
}
