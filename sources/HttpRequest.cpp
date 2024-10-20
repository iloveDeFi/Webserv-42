#include "HttpRequest.hpp"

std::string HttpRequest::trim(const std::string &str)
{
    size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first)
    {
        return "";
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

HttpRequest::HttpRequest(const std::string &rawData)
    : _method(""), _uri(""), _version("HTTP/1.1"), _headers(), _body(""), _queryParameters(""), _allowedMethods(initMethods())
{

    // rawData is good here!
    std::cout << "Raw request data: " << rawData << std::endl;

    std::istringstream requestStream(rawData);
    std::string requestLine;

    if (std::getline(requestStream, requestLine))
    {
        std::istringstream lineStream(requestLine);
        lineStream >> _method >> _uri >> _version;

        if (_method.empty() || _uri.empty() || _version.empty())
        {
            throw std::runtime_error("Invalid request line");
        }

        size_t queryPos = _uri.find('?');
        if (queryPos != std::string::npos)
        {
            _queryParameters = _uri.substr(queryPos + 1);
            _uri = _uri.substr(0, queryPos);
        }
    }

    std::string headerLine;
    while (std::getline(requestStream, headerLine) && !headerLine.empty())
    {
        size_t colonPos = headerLine.find(':');
        if (colonPos != std::string::npos)
        {
            std::string headerName = trim(headerLine.substr(0, colonPos));
            std::string headerValue = trim(headerLine.substr(colonPos + 1));
            _headers[headerName] = headerValue;
        }
    }

    std::map<std::string, std::string>::iterator contentLengthIt = _headers.find("Content-Length");
    if (contentLengthIt != _headers.end())
    {
        int contentLength = atoi(contentLengthIt->second.c_str());
        if (contentLength > 0)
        {
            char *bodyData = new char[contentLength + 1];
            requestStream.read(bodyData, contentLength);
            bodyData[contentLength] = '\0';
            _body = std::string(bodyData);
            delete[] bodyData;
        }
    }
}

HttpRequest::~HttpRequest() {}

HttpRequest::HttpRequest(const HttpRequest &src)
    : _method(src._method), _uri(src._uri), _version(src._version), _headers(src._headers), _body(src._body), _queryParameters(src._queryParameters), _allowedMethods(src._allowedMethods) {}

HttpRequest &HttpRequest::operator=(const HttpRequest &src)
{
    if (this != &src)
    {
        _method = src._method;
        _uri = src._uri;
        _version = src._version;
        _headers = src._headers;
        _body = src._body;
        _queryParameters = src._queryParameters;
        _allowedMethods = src._allowedMethods;
    }
    return *this;
}

std::string HttpRequest::getMethod() const { return _method; }
std::string HttpRequest::getURI() const { return _uri; }
std::string HttpRequest::getHTTPVersion() const { return _version; }

std::map<std::string, std::string> HttpRequest::getHeaders() const { return _headers; }

std::string HttpRequest::getHeader(const std::string &name) const
{
    std::map<std::string, std::string>::const_iterator it = _headers.find(name);
    if (it != _headers.end())
    {
        return it->second;
    }
    return "";
}

std::string HttpRequest::getBody() const { return _body; }
std::string HttpRequest::getQueryParameters() const { return _queryParameters; }

bool HttpRequest::isChunked() const { return false; }

std::set<std::string> HttpRequest::initMethods()
{
    std::set<std::string> methods;
    methods.insert("GET");
    methods.insert("POST");
    methods.insert("DELETE");
    return methods;
}

bool HttpRequest::isMethodAllowed(const std::string &method) const
{
    return _allowedMethods.find(method) != _allowedMethods.end();
}

bool HttpRequest::isSupportedContentType(const std::string &contentType) const
{
    std::set<std::string> supportedTypes;
    supportedTypes.insert("application/json");
    supportedTypes.insert("text/html");
    supportedTypes.insert("text/plain");

    return supportedTypes.find(contentType) != supportedTypes.end();
}

std::ostream &operator<<(std::ostream &os, const HttpRequest &req)
{
    os << "Method: " << req.getMethod() << "\n";
    os << "URI: " << req.getURI() << "\n";
    os << "Version: " << req.getHTTPVersion() << "\n";
    os << "Headers: \n";
    std::map<std::string, std::string> headers = req.getHeaders();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
    {
        os << it->first << ": " << it->second << "\n";
    }
    os << "Body: " << req.getBody() << "\n";
    os << "Query Parameters: " << req.getQueryParameters() << "\n";
    os << "Chunked: " << (req.isChunked() ? "Yes" : "No") << "\n";
    return os;
}

void HttpRequest::requestController(HttpResponse &response)
{
    HttpConfig::Location locationConfig;
    GetRequestHandler getHandler(locationConfig);
    PostRequestHandler postHandler(locationConfig);
    DeleteRequestHandler deleteHandler(locationConfig);
    UnknownRequestHandler unknownHandler(locationConfig);

    std::map<std::string, RequestController *> handlerMap;
    handlerMap["GET"] = &getHandler;
    handlerMap["POST"] = &postHandler;
    handlerMap["DELETE"] = &deleteHandler;

    std::string method = getMethod();
    std::map<std::string, RequestController *>::iterator it = handlerMap.find(method);

    if (it != handlerMap.end())
    {
        RequestController *handler = it->second;
        handler->handle(*this, response);
    }
    else
    {
        unknownHandler.handle(*this, response);
    }
}