#include "HttpRequest.hpp"

HttpRequest::HttpRequest() : _method(""), _uri(""), _version("HTTP/1.1") {}

HttpRequest::~HttpRequest() {}

HttpRequest::HttpRequest(const HttpRequest& src) : _method(src._method), _uri(src._uri), _version(src._version), _headers(src._headers), _body(src._body) {}

HttpRequest& HttpRequest::operator=(const HttpRequest& src) {
    if (this != &src) {
        this->_method = src._method;
        this->_uri = src._uri;
        this->_version = src._version;
        this->_headers = src._headers;
        this->_body = src._body;
        this->_is_chunked = src._is_chunked;
    }
    return *this;
}

// GETTERS
std::string HttpRequest::getMethod() const { return _method; }
std::string HttpRequest::getURI() const { return _uri; }
std::string HttpRequest::getHTTPVersion() const { return _version; }
std::map<std::string, std::string> HttpRequest::getHeaders() const { return _headers; }
std::string HttpRequest::getBody() const { return _body; }
bool HttpRequest::isChunked() const { return _is_chunked; }
std::string HttpRequest::getHeader(const std::string& name) const {
    auto it = _headers.find(name);
    return it != _headers.end() ? it->second : "";
}

// SETTERS
void HttpRequest::setMethod(const std::string& method) { _method = method; }
void HttpRequest::setURI(const std::string& uri) { _uri = uri; }
void HttpRequest::setHTTPVersion(const std::string& version) { _version = version; }
void HttpRequest::setHeaders(const std::map<std::string, std::string>& headers) { _headers = headers; }
void HttpRequest::setBody(const std::string& body) { _body = body; }
void HttpRequest::setIsChunked(bool is_chunked) { _is_chunked = is_chunked; }

// void HttpRequest::parse(const std::string& rawRequest) {
//     if (rawRequest.empty()) {
//         // Gérer le cas où la requête est vide
//         return;
//     }
    
//     size_t method_end = rawRequest.find(' ');
//     if (method_end == std::string::npos) {
//         // Gestion d'erreur : méthode non trouvée
//         return;
//     }
//     _method = rawRequest.substr(0, method_end);

//     size_t uri_start = method_end + 1;
//     size_t uri_end = rawRequest.find(' ', uri_start);
//     if (uri_end == std::string::npos) {
//         // Gestion d'erreur : URI non trouvée
//         return;
//     }
//     _uri = rawRequest.substr(uri_start, uri_end - uri_start);

//     size_t version_start = uri_end + 1;
//     size_t version_end = rawRequest.find("\r\n", version_start);
//     if (version_end == std::string::npos) {
//         // Gestion d'erreur : version non trouvée
//         return;
//     }
//     _version = rawRequest.substr(version_start, version_end - version_start);

//     // Parse headers (simplified)
//     size_t headers_start = rawRequest.find("\r\n") + 2;
//     size_t headers_end = rawRequest.find("\r\n\r\n");
//     if (headers_end == std::string::npos) {
//         // Gestion d'erreur : fin des en-têtes non trouvée
//         return;
//     }
//     std::string headers_str = rawRequest.substr(headers_start, headers_end - headers_start);

//     size_t pos = 0;
//     while ((pos = headers_str.find("\r\n")) != std::string::npos) {
//         std::string line = headers_str.substr(0, pos);
//         size_t colon = line.find(':');
//         if (colon == std::string::npos) {
//             // Gestion d'erreur : format d'en-tête incorrect
//             return;
//         }
//         std::string name = line.substr(0, colon);
//         std::string value = line.substr(colon + 2);  // +2 to skip ": "
//         _headers[name] = value;
//         headers_str.erase(0, pos + 2);
//     }

//     // Parse body (if any)
//     _body = rawRequest.substr(headers_end + 4);
// }

std::vector<std::string> HttpRequest::initMethods() {
    std::vector<std::string> vecMethods;

    vecMethods.push_back("GET");
    vecMethods.push_back("HEAD");
    vecMethods.push_back("POST");
    vecMethods.push_back("PUT");
    vecMethods.push_back("DELETE");
    vecMethods.push_back("CONNECT");
    vecMethods.push_back("OPTIONS");
    vecMethods.push_back("TRACE");
    vecMethods.push_back("PATCH");
    return vecMethods;
}
