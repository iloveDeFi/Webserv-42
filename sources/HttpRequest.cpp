#include "../includes/HttpRequest.hpp"
#include "../includes/HttpException.hpp"


// COPLIEN'S FORM
HttpRequest::HttpRequest() 
    : _method(""), _uri(""), _version("HTTP/1.1"), _headers(), _body(""), _queryParameters(), _isChunked(false), _allowedMethods(initMethods()) {}

HttpRequest::~HttpRequest() {}

HttpRequest::HttpRequest(const HttpRequest& src) 
    : _method(src._method), _uri(src._uri), _version(src._version), _headers(src._headers), _body(src._body), _queryParameters(src._queryParameters), _isChunked(src._isChunked), _allowedMethods(src._allowedMethods) {}

HttpRequest& HttpRequest::operator=(const HttpRequest& src) {
    if (this != &src) {
        _method = src._method;
        _uri = src._uri;
        _version = src._version;
        _headers = src._headers;
        _body = src._body;
        _queryParameters = src._queryParameters;
        _allowedMethods = src._allowedMethods;
        _isChunked = src._isChunked;
    }
    return *this;
}

// 1) REQUEST LINE
std::string HttpRequest::getMethod() const { return _method; }
std::string HttpRequest::getURI() const { return _uri; }
std::string HttpRequest::getHTTPVersion() const { return _version; }
void HttpRequest::setMethod(const std::string& method) {
	if (_allowedMethods.find(method) == _allowedMethods.end()) {
		throw HttpException("Invalid method: " + method);
	}
	_method = method;
}
void HttpRequest::setURI(const std::string& uri) { _uri = uri; }
void HttpRequest::setHTTPVersion(const std::string& version) {
	if (version != "HTTP/1.1" && version != "HTTP/1.0") {
		throw HttpException("Invalid HTTP version: " + version);
	}
	_version = version;
}

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
void HttpRequest::setHeaders(const std::map<std::string, std::string>& headers) { _headers = headers; }


// 3) BODY
std::string HttpRequest::getBody() const { return _body; }
std::map<std::string, std::string> HttpRequest::getQueryParameters() const { return _queryParameters; }
void HttpRequest::setBody(const std::string& body) {
	if (!body.empty() && body.size() > MAX_BODY_SIZE) {
		throw BodyTooLargeException();
	}
	_body = body;
}
void HttpRequest::setQueryParameters(const std::map<std::string, std::string>& query_params) { _queryParameters = query_params; }

// OTHER
const std::set<std::string> HttpRequest::initMethods() {
    std::set<std::string> methods;
    methods.insert("GET");
    methods.insert("POST");
    methods.insert("DELETE");
    return methods;
}
void HttpRequest::setIsChunked(bool isChunked) { _isChunked = isChunked; }

// Parsing principal
void HttpRequest::parse(const std::string& raw_request) {
    if (!isValidRequest(raw_request)) {
        throw HttpException("Invalid request format: request is not properly formatted.");
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
                throw HttpException("GET or DELETE request should not contain a body.");
            }
        }

        setIsChunked(checkIfChunked(raw_request));
        setQueryParameters(extractQueryParameters(_uri));
    } catch (const HttpException& e) {
        std::cerr << "Error during request parsing: " << e.what() << std::endl;
        clearRequestData();
    }
}

// Fonctions auxiliaires (trim, safe_substr, etc.)
std::string HttpRequest::safe_substr(const std::string& str, size_t start, size_t length) {
    if (start >= str.size()) throw HttpException("Substring extraction failed: start index out of bounds.");
    if (start + length > str.size()) throw HttpException("Substring extraction failed: length out of bounds.");
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

// Fonctions d'extraction
std::string HttpRequest::extractMethod(const std::string& raw_request) {
    size_t method_end = raw_request.find(' ');
    if (method_end == std::string::npos) throw MissingMethodException();
    return raw_request.substr(0, method_end);
}

std::string HttpRequest::extractURI(const std::string& raw_request) {
    size_t method_end = raw_request.find(' ');
    if (method_end == std::string::npos) throw MissingMethodException();

    size_t uri_end = raw_request.find(' ', method_end + 1);
    if (uri_end == std::string::npos) throw MissingURIException();

    std::string uri = safe_substr(raw_request, method_end + 1, uri_end - method_end - 1);
    if (uri.empty()) throw MissingURIException();

    return uri;
}

std::map<std::string, std::string> HttpRequest::extractHeaders(const std::string& raw_request) {
    std::map<std::string, std::string> headers;
    size_t headers_start = raw_request.find("\r\n");
    if (headers_start == std::string::npos) throw HttpException("Missing headers.");
    headers_start += 2;

    size_t headers_end = raw_request.find("\r\n\r\n", headers_start);
    if (headers_end == std::string::npos) throw HttpException("Missing end of headers.");

    size_t current = headers_start;
    while (current < headers_end) {
        size_t line_end = raw_request.find("\r\n", current);
        if (line_end == std::string::npos || line_end > headers_end) break;

        size_t colon_pos = raw_request.find(':', current);
        if (colon_pos == std::string::npos || colon_pos > line_end) {
            throw HttpException("Invalid header format: missing ':' in header.");
        }

        
        if (raw_request.find(':', colon_pos + 1) != std::string::npos && raw_request.find(':', colon_pos + 1) < line_end) {
            throw HttpException("Invalid header format.");
        }

        std::string key = safe_substr(raw_request, current, colon_pos - current);
        std::string value = safe_substr(raw_request, colon_pos + 1, line_end - colon_pos - 1);

        key = trim(key);
        value = trim(value);

        if (key.empty()) throw HttpException("Invalid header format: empty header key.");

        headers[key] = value;
        current = line_end + 2;
    }

    return headers;
}

std::string HttpRequest::extractBody(const std::string& raw_request) {
    size_t body_start = raw_request.find("\r\n\r\n");
    if (body_start == std::string::npos) return "";

    std::string body = safe_substr(raw_request, body_start + 4, std::string::npos);
    if (body.size() > MAX_BODY_SIZE) {
        throw BodyTooLargeException();
    }

    return body;
}

std::string HttpRequest::extractHTTPVersion(const std::string& raw_request) {
    size_t uri_end = raw_request.find(' ', raw_request.find(' ') + 1);
    if (uri_end == std::string::npos) throw MissingURIException();

    size_t version_end = raw_request.find("\r\n", uri_end + 1);
    if (version_end == std::string::npos) throw MissingHTTPVersionException();

    return safe_substr(raw_request, uri_end + 1, version_end - uri_end - 1);
}

/* aide à déterminer le mode de transfert des données dans une requête HTTP(en morceaux ou non), 
permettant au serveur de gérer correctement les données reçues.*/
bool HttpRequest::checkIfChunked(const std::string& raw_request) {
    std::map<std::string, std::string> headers = extractHeaders(raw_request);
    std::map<std::string, std::string>::const_iterator it = headers.find("Transfer-Encoding");
    if (it == headers.end()) return false;
    std::string encoding = trim(it->second);

    return encoding == "chunked";
}

bool HttpRequest::isChunked() const {
    return _isChunked;
}

std::map<std::string, std::string> HttpRequest::extractQueryParameters(const std::string& uri) {
    std::map<std::string, std::string> query_params;
    size_t query_start = uri.find('?');
    if (query_start == std::string::npos) return query_params;

    std::string query_string = uri.substr(query_start + 1);
    size_t param_start = 0;

    while (param_start < query_string.size()) {
        size_t param_end = query_string.find('&', param_start);
        if (param_end == std::string::npos) param_end = query_string.size();

        std::string param = query_string.substr(param_start, param_end - param_start);
        size_t equal_pos = param.find('=');

        if (equal_pos == std::string::npos) {
            throw InvalidQueryStringException();
        }

        std::string key = trim(param.substr(0, equal_pos));
        std::string value = trim(param.substr(equal_pos + 1));

        query_params[key] = value;
        param_start = param_end + 1;
    }

    return query_params;
}

// Surcharge de l'opérateur << pour afficher la requête
std::ostream& operator<<(std::ostream& os, const HttpRequest& req) {
    os << "Method: " << req.getMethod() << "\n";
    os << "URI: " << req.getURI() << "\n";
    os << "Version: " << req.getHTTPVersion() << "\n";
    os << "Headers: \n";
    std::map<std::string, std::string> headers = req.getHeaders();  // Copie locale des headers
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

    } catch (const HttpException& e) {
        std::cerr << "Error during request parsing: " << e.what() << std::endl;
    }
}
