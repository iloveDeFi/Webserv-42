#include "../includes/HttpRequest.hpp"
#include "../includes/HttpRequestException.hpp"

const size_t MAX_BODY_SIZE = 2 * 1024 * 1024; // 2 MB

// COPLIEN'S FORM
HttpRequest::HttpRequest() 
    : HttpMessage(), _method(""), _uri(""), _queryParameters(), _isChunked(false), _allowedMethods(initMethods()) {}

HttpRequest::~HttpRequest() {}

HttpRequest::HttpRequest(const HttpRequest& src) 
    : HttpMessage(src), _method(src._method), _uri(src._uri), _queryParameters(src._queryParameters), _isChunked(src._isChunked), _allowedMethods(src._allowedMethods) {}

HttpRequest& HttpRequest::operator=(const HttpRequest& src) {
    if (this != &src) {
        HttpMessage::operator=(src);
        _method = src._method;
        _uri = src._uri;
        _queryParameters = src._queryParameters;
        _allowedMethods = src._allowedMethods;
        _isChunked = src._isChunked;
    }
    return *this;
}

// Getters
std::string HttpRequest::getMethod() const { return _method; }
std::string HttpRequest::getURI() const { return _uri; }
const std::map<std::string, std::string>& HttpRequest::getQueryParameters() const { return _queryParameters; }
bool HttpRequest::isChunked() const { return _isChunked; }

// Setters
void HttpRequest::setMethod(const std::string& method) {
    if (_allowedMethods.find(method) == _allowedMethods.end()) {
        throw HttpRequestException("Invalid method: " + method);
    }
    _method = method;
}

void HttpRequest::setURI(const std::string& uri) { 
    _uri = uri;
    _queryParameters = extractQueryParameters(uri);
}

void HttpRequest::setIsChunked(bool isChunked) { _isChunked = isChunked; }

void HttpRequest::setQueryParameters(const std::map<std::string, std::string>& query_params) { _queryParameters = query_params; }

void HttpRequest::setHeader(const std::string& name, const std::string& value) {
    HttpMessage::setHeader(name, value);
}

// OTHER
std::set<std::string> HttpRequest::initMethods() {
    std::set<std::string> methods;
    methods.insert("GET");
    methods.insert("POST");
    methods.insert("DELETE");
    return methods;
}

// Parsing principal
void HttpRequest::parse(const std::string& raw_request) {
    if (!isValidRequest(raw_request)) {
        throw HttpRequestException("Invalid request format: request is not properly formatted.");
    }

    try {
        setMethod(extractMethod(raw_request));
        setURI(extractURI(raw_request));
        setHTTPVersion(extractHTTPVersion(raw_request));
        setHeaders(extractHeaders(raw_request));
        
        if (_method == "POST") {
            setBody(extractBody(raw_request));
        } else if (_method == "GET" || _method == "DELETE") {
            if (hasBody(raw_request)) {
                throw HttpRequestException("GET or DELETE request should not contain a body.");
            }
        }

        setIsChunked(checkIfChunked(raw_request));
        setQueryParameters(extractQueryParameters(_uri));
    } catch (const HttpRequestException& e) {
        std::cerr << "Error during request parsing: " << e.what() << std::endl;
        clearRequestData();
    }
}

// Fonctions auxiliaires (trim, safe_substr, etc.)
std::string HttpRequest::safe_substr(const std::string& str, size_t start, size_t length) {
    if (start >= str.size()) throw HttpRequestException("Substring extraction failed: start index out of bounds.");
    if (start + length > str.size()) throw HttpRequestException("Substring extraction failed: length out of bounds.");
    return str.substr(start, length);
}

std::string HttpRequest::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return safe_substr(str, first, last - first + 1);
}

bool HttpRequest::isValidRequest(const std::string& raw_request) {
    size_t method_end = raw_request.find(' ');
    size_t uri_end = raw_request.find(' ', method_end + 1);
    size_t version_end = raw_request.find("\r\n", uri_end + 1);
    if (method_end == std::string::npos || uri_end == std::string::npos || version_end == std::string::npos) {
        return false;
    }
    return true;
}

bool HttpRequest::hasBody(const std::string& raw_request) {
    size_t body_start = raw_request.find("\r\n\r\n");
    if (body_start == std::string::npos) return false;
    return (raw_request.size() > body_start + 4);
}

void HttpRequest::clearRequestData() {
    _method.clear();
    _uri.clear();
    _queryParameters.clear();
    _version.clear();
    _headers.clear();
    _body.clear();
    _isChunked = false;
}


// Surcharge de l'opérateur << pour afficher la requête
std::ostream& operator<<(std::ostream& os, const HttpRequest& req) {
    os << "Method: " << req.getMethod() << "\n";
    os << "URI: " << req.getURI() << "\n";
    os << "Version: " << req.getHTTPVersion() << "\n";
    os << "Headers: \n";
    const std::map<std::string, std::string>& headers = req.getHeaders();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        os << "  " << it->first << ": " << it->second << "\n";
    }
    os << "Body: " << req.getBody() << "\n";
    return os;
}

/*Fonction pour tester mon parsing de la requête (peut être supprimée par la suite)*/
void testRequest(const std::string& raw_request) {
    HttpRequest request;
    try {
        request.parse(raw_request);

        
        std::cout << "Method: " << request.getMethod() << std::endl;
        std::cout << "URI: " << request.getURI() << std::endl;
        std::cout << "HTTP Version: " << request.getHTTPVersion() << std::endl;
        std::cout << "Headers:" << std::endl;

        std::map<std::string, std::string> headers = request.getHeaders();
        for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
            std::cout << it->first << ": " << it->second << std::endl;
        }

        std::cout << "Body: " << request.getBody() << std::endl;

        if (request.isChunked()) {
            std::cout << "The request is chunked." << std::endl;
        } else {
            std::cout << "The request is not chunked." << std::endl;
        }

    } catch (const HttpRequestException& e) {
        std::cerr << "Error during request parsing: " << e.what() << std::endl;
    }
}