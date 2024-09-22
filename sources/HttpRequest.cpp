#include "HttpRequest.hpp"
#include <sstream>

HttpRequest::HttpRequest() : AHttpMessage(), _method(""), _uri(""), _queryParameters(), _allowedMethods(initMethods()) {}

HttpRequest::~HttpRequest() {}

HttpRequest::HttpRequest(const HttpRequest& src) 
    : AHttpMessage(src), _method(src._method), _uri(src._uri), _queryParameters(src._queryParameters), _allowedMethods(src._allowedMethods) {}

HttpRequest& HttpRequest::operator=(const HttpRequest& src) {
    if (this != &src) {
        AHttpMessage::operator=(src);
        _method = src._method;
        _uri = src._uri;
        _queryParameters = src._queryParameters;
        _allowedMethods = src._allowedMethods;
    }
    return *this;
}


void HttpRequest::parse(const std::string& raw_request) {
    if (!isValidRequest(raw_request)) {
        throw HttpException("Invalid request format: request is not properly formatted.");
    }

    try {
        setMethod(extractMethod(raw_request));
        setURI(extractURI(raw_request));
        setVersion(extractVersion(raw_request));
        setHeaders(extractHeaders(raw_request));
        
        if (_method == "POST") {
            setBody(extractBody(raw_request));
        } else if (_method == "GET" || _method == "DELETE") {
            if (!getBody().empty()) {
                throw HttpException("GET or DELETE request should not contain a body.");
            }
        }

        setIsChunked(checkIfChunked(raw_request));
        setQueryParameters(extractQueryParameters(_uri));
    } catch (const HttpException& e) {
        std::cerr << "Error during request parsing: " << e.what() << std::endl;
        clearMessageData();
    }
}

const std::set<std::string> HttpRequest::initMethods() {
    std::set<std::string> methods;
    methods.insert("GET");
    methods.insert("POST");
    methods.insert("DELETE");
    return methods;
}

std::string HttpRequest::extractMethod(const std::string& raw_request) {
    size_t method_end = raw_request.find(' ');
    if (method_end == std::string::npos) throw HttpException("Missing method.");
    return raw_request.substr(0, method_end);
}

std::string HttpRequest::extractURI(const std::string& raw_request) {
    size_t method_end = raw_request.find(' ');
    if (method_end == std::string::npos) throw HttpException("Missing method.");

    size_t uri_end = raw_request.find(' ', method_end + 1);
    if (uri_end == std::string::npos) throw HttpException("Missing URI.");

    std::string uri = safe_substr(raw_request, method_end + 1, uri_end - method_end - 1);
    if (uri.empty()) throw HttpException("Missing URI.");

    return uri;
}

std::map<std::string, std::string> HttpRequest::extractQueryParameters(const std::string& uri) {
    std::map<std::string, std::string> query_params;
    size_t query_start = uri.find('?');
    if (query_start == std::string::npos) return query_params;

    std::string query_string = uri.substr(query_start + 1);
    std::istringstream query_stream(query_string);
    std::string param;

    while (std::getline(query_stream, param, '&')) {
        size_t equal_pos = param.find('=');
        if (equal_pos == std::string::npos) {
            throw HttpException("Invalid query string.");
        }

        std::string key = trim(param.substr(0, equal_pos));
        std::string value = trim(param.substr(equal_pos + 1));

        query_params[key] = value;
    }

    return query_params;
}

bool HttpRequest::isValidRequest(const std::string& raw_request) {
    size_t method_end = raw_request.find(' ');
    size_t uri_end = raw_request.find(' ', method_end + 1);
    size_t version_end = raw_request.find("\r\n", uri_end + 1);
    return (method_end != std::string::npos && uri_end != std::string::npos && version_end != std::string::npos);
}

// Getters
std::string HttpRequest::getMethod() const { return _method; }
std::string HttpRequest::getURI() const { return _uri; }
std::map<std::string, std::string> HttpRequest::getQueryParameters() const { return _queryParameters; }

// Setters
void HttpRequest::setMethod(const std::string& method) {
    if (_allowedMethods.find(method) == _allowedMethods.end()) {
        throw HttpException("Invalid method: " + method);
    }
    _method = method;
}
void HttpRequest::setURI(const std::string& uri) { _uri = uri; }
void HttpRequest::setQueryParameters(const std::map<std::string, std::string>& query_params) { _queryParameters = query_params; }

std::ostream& operator<<(std::ostream& os, const HttpRequest& req) {
    os << "Method: " << req.getMethod() << "\n";
    os << "URI: " << req.getURI() << "\n";
    os << "Version: " << req.getVersion() << "\n";
    os << "Headers: \n";
    std::map<std::string, std::string> headers = req.getHeaders();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        os << "  " << it->first << ": " << it->second << "\n";
    }
    os << "Body: " << req.getBody() << "\n";
    os << "Query Parameters: \n";
    std::map<std::string, std::string> params = req.getQueryParameters();
    for (std::map<std::string, std::string>::const_iterator it = params.begin(); it != params.end(); ++it) {
        os << "  " << it->first << " = " << it->second << "\n";
    }
    return os;
}