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
    }
    return *this;
}

std::string HttpRequest::getHeader(const std::string& name) const {
    auto it = _headers.find(name);
    return it != _headers.end() ? it->second : "";
}

void HttpRequest::parse(const std::string& rawRequest) {
    if (rawRequest.empty()) {
        // Gérer le cas où la requête est vide
        return;
    }
    
    size_t method_end = rawRequest.find(' ');
    if (method_end == std::string::npos) {
        // Gestion d'erreur : méthode non trouvée
        return;
    }
    _method = rawRequest.substr(0, method_end);

    size_t uri_start = method_end + 1;
    size_t uri_end = rawRequest.find(' ', uri_start);
    if (uri_end == std::string::npos) {
        // Gestion d'erreur : URI non trouvée
        return;
    }
    _uri = rawRequest.substr(uri_start, uri_end - uri_start);

    size_t version_start = uri_end + 1;
    size_t version_end = rawRequest.find("\r\n", version_start);
    if (version_end == std::string::npos) {
        // Gestion d'erreur : version non trouvée
        return;
    }
    _version = rawRequest.substr(version_start, version_end - version_start);

    // Parse headers (simplified)
    size_t headers_start = rawRequest.find("\r\n") + 2;
    size_t headers_end = rawRequest.find("\r\n\r\n");
    if (headers_end == std::string::npos) {
        // Gestion d'erreur : fin des en-têtes non trouvée
        return;
    }
    std::string headers_str = rawRequest.substr(headers_start, headers_end - headers_start);

    size_t pos = 0;
    while ((pos = headers_str.find("\r\n")) != std::string::npos) {
        std::string line = headers_str.substr(0, pos);
        size_t colon = line.find(':');
        if (colon == std::string::npos) {
            // Gestion d'erreur : format d'en-tête incorrect
            return;
        }
        std::string name = line.substr(0, colon);
        std::string value = line.substr(colon + 2);  // +2 to skip ": "
        _headers[name] = value;
        headers_str.erase(0, pos + 2);
    }

    // Parse body (if any)
    _body = rawRequest.substr(headers_end + 4);
}
