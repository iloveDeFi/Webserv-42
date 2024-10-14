#include "../includes/HttpRequest.hpp"
#include "../includes/HttpRequestException.hpp"
#include <algorithm>
#include <sstream>

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

// GETTERS AND SETTERS
std::string HttpRequest::getMethod() const { return _method; }
std::string HttpRequest::getURI() const { return _uri; }
std::string HttpRequest::getHTTPVersion() const { return _version; }
std::map<std::string, std::string> HttpRequest::getHeaders() const { return _headers; }
std::string HttpRequest::getBody() const { return _body; }
std::map<std::string, std::string> HttpRequest::getQueryParameters() const { return _queryParameters; }

void HttpRequest::setMethod(const std::string& method) {
    if (_allowedMethods.find(method) == _allowedMethods.end()) {
        throw HttpRequestException("Invalid method: " + method);
    }
    _method = method;
}

void HttpRequest::setURI(const std::string& uri) { _uri = uri; }

void HttpRequest::setHTTPVersion(const std::string& version) {
    if (version != "HTTP/1.1" && version != "HTTP/1.0") {
        throw HttpRequestException("Invalid HTTP version: " + version);
    }
    _version = version;
}

void HttpRequest::setHeaders(const std::map<std::string, std::string>& headers) { _headers = headers; }

void HttpRequest::setBody(const std::string& body) {
    if (!body.empty() && body.size() > MAX_BODY_SIZE) {
        throw BodyTooLargeException();
    }
    _body = body;
}

void HttpRequest::setQueryParameters(const std::map<std::string, std::string>& query_params) { _queryParameters = query_params; }

void HttpRequest::setIsChunked(bool isChunked) { _isChunked = isChunked; }

// MAIN PARSING FUNCTION
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
        throw;
    }
}

// UTILITY FUNCTIONS
const std::set<std::string> HttpRequest::initMethods() {
    std::set<std::string> methods;
    methods.insert("GET");
    methods.insert("POST");
    methods.insert("DELETE");
    return methods;
}

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

std::string HttpRequest::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
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

bool HttpRequest::isChunked() const {
    return _isChunked;
}

// EXTRACTION FUNCTIONS
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
    if (headers_start == std::string::npos) throw HttpRequestException("Missing headers.");
    headers_start += 2;

    size_t headers_end = raw_request.find("\r\n\r\n", headers_start);
    if (headers_end == std::string::npos) throw HttpRequestException("Missing end of headers.");

    size_t current = headers_start;
    while (current < headers_end) {
        size_t line_end = raw_request.find("\r\n", current);
        if (line_end == std::string::npos || line_end > headers_end) break;

        std::string header_line = raw_request.substr(current, line_end - current);
        size_t colon_pos = header_line.find(':');
        if (colon_pos == std::string::npos) {
            throw HttpRequestException("Invalid header format: missing ':' in header '" + header_line + "'");
        }

        std::string key = toLower(trim(header_line.substr(0, colon_pos)));
        std::string value = trim(header_line.substr(colon_pos + 1));

        if (key.empty()) {
            throw HttpRequestException("Invalid header format: empty header key in '" + header_line + "'");
        }

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

// Dans HttpRequest.cpp, modifiez la méthode extractHTTPVersion comme suit:
std::string HttpRequest::extractHTTPVersion(const std::string& raw_request) {
    size_t first_space = raw_request.find(' ');
    if (first_space == std::string::npos) throw MissingMethodException();

    size_t second_space = raw_request.find(' ', first_space + 1);
    if (second_space == std::string::npos) throw MissingURIException();

    size_t version_end = raw_request.find("\r\n", second_space + 1);
    if (version_end == std::string::npos) throw MissingHTTPVersionException();

    std::string version = trim(raw_request.substr(second_space + 1, version_end - second_space - 1));
    
    if (version.substr(0, 5) != "HTTP/") {
        throw HttpRequestException("Invalid HTTP version format: '" + version + "'. Expected format: HTTP/x.x");
    }

    return version;
}

bool HttpRequest::checkIfChunked(const std::string& raw_request) {
    std::map<std::string, std::string> headers = extractHeaders(raw_request);
    std::map<std::string, std::string>::const_iterator it = headers.find("transfer-encoding");
    if (it == headers.end()) return false;
    std::string encoding = trim(it->second);

    return toLower(encoding) == "chunked";
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

std::string HttpRequest::getHeader(const std::string& name) const {
    std::map<std::string, std::string>::const_iterator it = _headers.find(toLower(name));
    return (it != _headers.end()) ? it->second : "";
}

// OPERATOR OVERLOADING
std::ostream& operator<<(std::ostream& os, const HttpRequest& req) {
    os << "Method: " << req.getMethod() << "\n";
    os << "URI: " << req.getURI() << "\n";
    os << "Version: " << req.getHTTPVersion() << "\n";
    os << "Headers: \n";
    std::map<std::string, std::string> headers = req.getHeaders();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        os << "  " << it->first << ": " << it->second << "\n";
    }
    os << "Body: " << req.getBody() << "\n";
    return os;
}

// TEST FUNCTION
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
        // Afficher les premières lignes de la requête pour plus de contexte
        std::istringstream iss(raw_request);
        std::string line;
        int lineCount = 0;
        std::cerr << "Request preview:" << std::endl;
        while (std::getline(iss, line) && lineCount < 3) {
            std::cerr << line << std::endl;
            lineCount++;
        }
        if (lineCount == 3) std::cerr << "..." << std::endl;
    }
}

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cstring>

void HttpRequest::executeCGI(const HttpConfig::Location& location, std::string& response) {
    int input_pipe[2];
    int output_pipe[2];

    if (pipe(input_pipe) < 0 || pipe(output_pipe) < 0) {
        throw std::runtime_error("Failed to create pipes for CGI execution");
    }

    pid_t pid = fork();

    if (pid == 0) {  // Child process
        // Set up environment variables
        setenv("REQUEST_METHOD", _method.c_str(), 1);
        setenv("QUERY_STRING", getQueryString().c_str(), 1);
        setenv("CONTENT_TYPE", getHeader("Content-Type").c_str(), 1);
        setenv("CONTENT_LENGTH", getHeader("Content-Length").c_str(), 1);
        // Add more environment variables as needed

        // Redirect stdin to input pipe
        dup2(input_pipe[0], STDIN_FILENO);
        close(input_pipe[1]);

        // Redirect stdout to output pipe
        dup2(output_pipe[1], STDOUT_FILENO);
        close(output_pipe[0]);

        // Execute the CGI script
        std::string script_path = location.root + _uri;
        execl(location.cgiHandler.c_str(), location.cgiHandler.c_str(), script_path.c_str(), NULL);

        // If execl fails
        exit(1);
    } else if (pid > 0) {  // Parent process
        close(input_pipe[0]);
        close(output_pipe[1]);

        // Write request body to input pipe
        write(input_pipe[1], _body.c_str(), _body.length());
        close(input_pipe[1]);

        // Read CGI output from output pipe
        char buffer[4096];
        ssize_t bytes_read;
        while ((bytes_read = read(output_pipe[0], buffer, sizeof(buffer))) > 0) {
            response.append(buffer, bytes_read);
        }
        close(output_pipe[0]);

        // Wait for child process to finish
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            throw std::runtime_error("CGI script execution failed");
        }
    } else {
        throw std::runtime_error("Failed to fork for CGI execution");
    }
}

std::string HttpRequest::getQueryString() const {
    std::string query_string;
    for (std::map<std::string, std::string>::const_iterator it = _queryParameters.begin(); it != _queryParameters.end(); ++it) {
        if (!query_string.empty()) {
            query_string += "&";
        }
        query_string += it->first + "=" + it->second;
    }
    return query_string;
}

void HttpRequest::testCGI() {
    HttpRequest request;
    HttpConfig::Location location;
    location.root = "/path/to/your/cgi-scripts";
    location.cgiHandler = "/usr/bin/python3";

    // Test GET request
    std::string get_request = "GET /cgi-bin/test.py?name=John HTTP/1.1\r\n"
                              "Host: localhost:8080\r\n"
                              "\r\n";
    request.parse(get_request);
    
    std::string response;
    try {
        request.executeCGI(location, response);
        std::cout << "CGI GET Response:\n" << response << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "CGI GET execution failed: " << e.what() << std::endl;
    }

    // Test POST request
    std::string post_request = "POST /cgi-bin/test.py HTTP/1.1\r\n"
                               "Host: localhost:8080\r\n"
                               "Content-Type: application/x-www-form-urlencoded\r\n"
                               "Content-Length: 9\r\n"
                               "\r\n"
                               "name=Jane";
    request.parse(post_request);
    
    try {
        request.executeCGI(location, response);
        std::cout << "CGI POST Response:\n" << response << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "CGI POST execution failed: " << e.what() << std::endl;
    }
}