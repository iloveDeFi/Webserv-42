#include "HttpResponse.hpp"

HttpResponse::HttpResponse()
    : _httpVersion("HTTP/1.1"), _statusCode(200), _reasonMessage("OK"), _headers(), _body(""), _isChunked(false) {}

HttpResponse::~HttpResponse() {}

HttpResponse::HttpResponse(const HttpResponse &src)
    : _httpVersion(src._httpVersion), _statusCode(src._statusCode), _reasonMessage(src._reasonMessage), _headers(src._headers), _body(src._body), _isChunked(src._isChunked) {}

HttpResponse &HttpResponse::operator=(const HttpResponse &src)
{
    if (this != &src)
    {
        _httpVersion = src._httpVersion;
        _statusCode = src._statusCode;
        _reasonMessage = src._reasonMessage;
        _headers = src._headers;
        _body = src._body;
        _isChunked = src._isChunked;
    }
    return *this;
}

std::string HttpResponse::readFile(const std::string &filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    file.close();

    return buffer.str();
}

void HttpResponse::generate200OK(const std::string &contentType, const std::string &bodyContent)
{
    setStatusCode(200);
    setReasonMessage("OK");

    setHeader("Content-Type", contentType);

    // TO DO : fix ensure function reset content-length to 0
    // ensureContentLength();
    setHeader("Content-Length", to_string(bodyContent.size()));
    setHeader("Cache-Control", "no-store");

    setBody(bodyContent);
}

void HttpResponse::generate201Created(const std::string &location)
{
    setStatusCode(201);
    setReasonMessage("Created");
    setHeader("Location", location);
    setHeader("Content-Type", "text/plain");
    std::string body = "201 Created : Resource successfully created at " + location;
    setBody(body);
    setHeader("Content-Length", to_string(body.size()));
}

void HttpResponse::generate202Accepted(const std::string &deletionInfo)
{
    setStatusCode(202);
    setReasonMessage("Accepted");
    setHeader("Content-Type", "text/plain");
    std::string body = "202 Accepted: The deletion request was accepted and is being processed.\n" + deletionInfo;
    setBody(body);
    setHeader("Content-Length", to_string(body.size()));
}

void HttpResponse::generate204NoContent(const std::string &errorMessage)
{
    setStatusCode(204);
    setReasonMessage("No Content");
    setHeader("Content-Type", "text/plain");
    setHeader("Content-Length", "0");
    setBody("");
    (void)errorMessage;
}

void HttpResponse::generate301MovedPermanently(const std::string &location)
{
    setStatusCode(301);
    setReasonMessage("Moved Permanently");
    setHeader("Location", location);
    setHeader("Content-Type", "text/plain");

    std::string body = "301 Moved Permanently: The requested resource has been moved permanently to " + location;
    setBody(body);
    setHeader("Content-Length", std::to_string(body.size()));
}

void HttpResponse::generate400BadRequest(const std::string &errorMessage /* , std::string root */)
{
    setStatusCode(400);
    setReasonMessage("Bad Request");
    setHeader("Content-Type", "text/plain");
    std::string body = "400 Bad Request: " + errorMessage;
    setBody(body);
    setHeader("Content-Length", to_string(body.size()));
}

void HttpResponse::generate403Forbidden(const std::string &errorMessage, const std::string &root)
{
    setStatusCode(403);
    setReasonMessage("Forbidden");
    setHeader("Content-Type", "text/html");

    std::string readError = root + "/errors/403.html";
    std::string body = readFile(readError);

    if (body.empty())
    {
        std::cerr << "Warning: Could not read 403.html, using default error message." << std::endl;
        body = "<html><body><h1>403 Forbidden</h1><p>" + errorMessage + "</p></body></html>";
    }

    setBody(body);
    setHeader("Content-Length", std::to_string(body.size()));
}

void HttpResponse::generate404NotFound(const std::string &errorMessage, const std::string &root)
{
    (void)root;
    Logger &logger = Logger::getInstance("server.log");
    setStatusCode(404);
    setReasonMessage("Not Found");
    setHeader("Content-Type", "text/html");

    std::string readError = root + "/errors/404.html";
    std::string body = readFile(readError);
    logger.log(" \n body is " + body);

    if (body.empty())
    {
        std::cerr << "Warning: Could not read 404.html, using default error message." << std::endl;
        body = "<html><body><h1>404 Not Found</h1><p>" + errorMessage + "</p></body></html>";
    }

    setBody(body);
    setHeader("Content-Length", std::to_string(body.size()));
}

void HttpResponse::generate405MethodNotAllowed(const std::string &allowedMethods)
{
    setStatusCode(405);
    setReasonMessage("Method Not Allowed");
    setHeader("Content-Type", "text/plain");
    std::string body = "405 Method Not Allowed: The method used is not allowed for this resource.\nAllowed Methods: " + allowedMethods;
    setBody(body);
    setHeader("Content-Length", to_string(body.size()));
    setHeader("Allow", allowedMethods);
}

void HttpResponse::generate409Conflict(const std::string &conflictInfo)
{
    setStatusCode(409);
    setReasonMessage("Conflict");
    setHeader("Content-Type", "text/plain");
    std::string body = conflictInfo;
    setBody(body);
    setHeader("Content-Length", to_string(body.size()));
}

void HttpResponse::generate413PayloadTooLarge(size_t size)
{
    setStatusCode(413);
    setReasonMessage("Payload too large");
    setHeader("Content-Type", "text/plain");
    std::string body = "413 Payload too large: max size is " + to_string(size);
    setBody(body);
    setHeader("Content-Length", to_string(body.size()));
}

void HttpResponse::generate415UnsupportedMediaType(const std::string &contentType)
{
    setStatusCode(415);
    setReasonMessage("Unsupported Media Type");
    setHeader("Content-Type", "text/plain");

    std::string body = "415 Unsupported Media Type: The media type '" + contentType;
    setBody(body);

    setHeader("Content-Length", std::to_string(body.size()));
}


void HttpResponse::generate500InternalServerError(const std::string &errorMessage, const std::string &root)
{
    (void)root;
    setStatusCode(500);
    setReasonMessage("Internal Server Error");
    setHeader("Content-Type", "text/html");
    std::string readError = root + "/errors/500.html";
    std::string body = readFile(readError);
    if (body.empty())
    {
        std::cerr << "Warning: Could not read 500.html, using default error message." << std::endl;
        body = "<html><body><h1>500 Internal Server Error</h1><p>The server encountered an error. " + errorMessage + "</p></body></html>";
    }
    setBody(body);
    setHeader("Content-Length", std::to_string(body.size()));
}

void HttpResponse::generate501NotImplemented(const std::string &errorMessage)
{
    setStatusCode(501);
    setReasonMessage("Not Implemented");
    setHeader("Content-Type", "text/plain");
    std::string body = errorMessage;
    setBody(body);
    setHeader("Content-Length", to_string(body.size()));
}

std::string HttpResponse::getFullResponse()
{
    std::string response;

    response += _httpVersion + " " + to_string(_statusCode) + " " + _reasonMessage + "\r\n";

    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
    {
        response += it->first + ": " + it->second + "\r\n";
    }

    response += "\r\n";

    response += _body;

    return response;
}

void HttpResponse::setHTTPVersion(const std::string &httpVersion) { _httpVersion = httpVersion; }
void HttpResponse::setStatusCode(int statusCode) { _statusCode = statusCode; }
void HttpResponse::setReasonMessage(const std::string &reasonMessage) { _reasonMessage = reasonMessage; }
void HttpResponse::setHeader(const std::string &name, const std::string &value) { _headers[name] = value; }
void HttpResponse::setHeaders(const std::map<std::string, std::string> &headers) { _headers = headers; }
void HttpResponse::setBody(const std::string &body) { _body = body; }
void HttpResponse::setIsChunked(bool isChunked) { _isChunked = isChunked; }

int HttpResponse::getStatusCode() const
{
    return _statusCode;
}

std::string HttpResponse::normalizeHeader(const std::string &header)
{
    std::string normalized = header;
    for (std::string::size_type i = 0; i < header.size(); ++i)
    {
        normalized[i] = std::tolower(header[i]);
    }
    return normalized;
}

void HttpResponse::ensureContentLength()
{
    if (_headers.find("Content-Length") == _headers.end())
    {
        _headers["Content-Length"] = to_string(_body.size());
    }
}

std::string HttpResponse::toString()
{
    std::string response = _httpVersion + " " + to_string(_statusCode) + " " + _reasonMessage + "\r\n";

    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
    {
        response += it->first + ": " + it->second + "\r\n";
    }

    response += "\r\n" + _body;
    return response;
}

std::string HttpResponse::generate404Error(const std::string &uri)
{
    std::string errorPage = "<html><body><h1>404 Not Found</h1>";
    errorPage += "<p>The requested resource " + uri + " was not found on this server.</p></body></html>";

    return "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: " + to_string(errorPage.size()) + "\r\n\r\n" + errorPage;
}

std::string HttpResponse::generateRedirection(const std::string &newUri)
{
    return "HTTP/1.1 302 Found\r\nLocation: " + newUri + "\r\n\r\n";
}

std::ostream &HttpResponse::print(std::ostream &os) const
{
    os << "--- RESPONSE LINE INFOS: ---" << std::endl;
    os << "httpVersion is: " << _httpVersion << " Status code is: " << _statusCode << " Reason Message is: " << _reasonMessage << "\n\n";

    os << "--- HEADER INFOS: ---" << std::endl;
    os << "Headers are: " << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
    {
        os << it->first << ": " << it->second << std::endl;
    }

    os << "--- BODY INFOS: ---" << std::endl;
    os << "Content-Length: " << _body.size() << "\n\n";
    os << "Body is: " << _body << "\n";
    os << "Body chunked: " << _isChunked << "\n";
    return os;
}

void HttpResponse::logHttpResponse(Logger &logger)
{
    std::ostringstream logMessage;

    logMessage << "HTTP Response:\n";
    logMessage << "HTTP Version: " << _httpVersion << "\n";
    logMessage << "Status Code: " << _statusCode << "\n";
    logMessage << "Reason Message: " << _reasonMessage << "\n";

    logMessage << "Headers:\n";
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
    {
        logMessage << it->first << ": " << it->second << "\n";
    }

    logMessage << "Body: " << _body << "\n";
    (void)logger;
    // logger.log(logMessage.str());
}

std::string HttpResponse::getBody() const
{
    return (_body);
}