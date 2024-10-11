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

    // RESPONSE
    // 200 OK response
void HttpResponse::generate200OK(const std::string& contentType, const std::string& bodyContent) {
    setStatusCode(200);
    setReasonMessage("OK");
    setHeader("Content-Type", contentType);
    setBody(bodyContent);
    ensureContentLength();
    setHeader("Content-Length", std::to_string(bodyContent.size()));
}

// 201 Created response
void HttpResponse::generate201Created(const std::string& location) {
    setStatusCode(201);
    setReasonMessage("Created");
    setHeader("Location", location); // URI of the newly created resource
    setHeader("Content-Type", "text/plain");
    std::string body = "Resource created at " + location;
    setBody(body);
    setHeader("Content-Length", std::to_string(body.size()));
}

// 202 Accepted response
void HttpResponse::generate202Accepted(const std::string& deletionInfo) {
    setStatusCode(202);
    setReasonMessage("Accepted");
    setHeader("Content-Type", "text/plain");
    std::string body = "202 Accepted: The deletion request was accepted and is being processed.\n" + deletionInfo;
    setBody(body);
    setHeader("Content-Length", std::to_string(body.size()));
}


// 204 No Content response
void HttpResponse::generate204NoContent() {
    setStatusCode(204);
    setReasonMessage("No Content");
    setHeader("Content-Type", "text/plain");
    setBody(""); // No body
}

// 403 Forbidden response
void HttpResponse::generate403Forbidden() {
    setStatusCode(403);
    setReasonMessage("Forbidden");
    setHeader("Content-Type", "text/plain");
    std::string body = "403 Forbidden: You don't have permission to access this resource.";
    setBody(body);
    setHeader("Content-Length", std::to_string(body.size()));
}

// 404 Not Found response
void HttpResponse::generate404NotFound() {
    setStatusCode(404);
    setReasonMessage("Not Found");
    setHeader("Content-Type", "text/plain");
    std::string body = "404 Not Found: The requested resource could not be found.";
    setBody(body);
    setHeader("Content-Length", std::to_string(body.size()));
}

// 409 Conflict response for PUT method
void HttpResponse::generate409Conflict(const std::string& conflictInfo) {
    setStatusCode(409);
    setReasonMessage("Conflict");
    setHeader("Content-Type", "text/plain");
    std::string body = "409 Conflict: The request could not be completed due to a conflict with the current state of the resource.\n" + conflictInfo;
    setBody(body);
    setHeader("Content-Length", std::to_string(body.size()));
}

// 500 Internal Server Error response
void HttpResponse::generate500InternalServerError() {
    setStatusCode(500);
    setReasonMessage("Internal Server Error");
    setHeader("Content-Type", "text/plain");
    std::string body = "500 Internal Server Error: The server encountered an error.";
    setBody(body);
    setHeader("Content-Length", std::to_string(body.size()));
}

// TO DO : check if add getters here
std::string HttpResponse::getFullResponse() {
    std::string response;

    // Ajouter la ligne de statut (version HTTP, code et message)
    response += "HTTP/1.1 " + std::to_string(getStatusCode()) + " " + getReasonMessage() + "\r\n";

    // Ajouter les headers
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        response += it->first + ": " + it->second + "\r\n";
    }

    // Ajouter une ligne vide entre les headers et le corps
    response += "\r\n";

    // Ajouter le corps
    response += getBody();

    return response;
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
std::string HttpResponse::normalizeHeader(const std::string& header) {
    std::string normalized = header;
    for (std::string::size_type i = 0; i < header.size(); ++i) {
        normalized[i] = std::tolower(header[i]);
    }
    return normalized;
}

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
