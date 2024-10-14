#include "HttpRequest.hpp"

// COPLIEN'S FORM
HttpRequest::HttpRequest(const std::string &rawData)
    : _method(""), _uri(""), _version("HTTP/1.1"), _headers(), _body(""), _queryParameters(""), _allowedMethods(initMethods()) /* , _cookies(""), _isChunked(false) */ {}

HttpRequest::~HttpRequest() {}

HttpRequest::HttpRequest(const HttpRequest &src)
    : _method(src._method), _uri(src._uri), _version(src._version), _headers(src._headers), _body(src._body), _queryParameters(src._queryParameters), _allowedMethods(src._allowedMethods) /* _cookies(src._cookies), _isChunked(src._isChunked) */ {}

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
        // BONUS :
        //_cookies = src._cookies;
    }
    return *this;
}

// 1) REQUEST LINE
std::string HttpRequest::getMethod() const { return _method; }
std::string HttpRequest::getURI() const { return _uri; }
std::string HttpRequest::getHTTPVersion() const { return _version; }

// 2) HEADERS
std::map<std::string, std::string> HttpRequest::getHeaders() const { return _headers; }
std::string HttpRequest::getHeader(const std::string &name) const
{
    std::map<std::string, std::string>::const_iterator it = _headers.find(name);
    if (it != _headers.end())
    {
        return it->second;
    }
    else
    {
        return "";
    }
}

// TO DO LATER ALIGATOR
// accept
// accept-language
// accept-encoding
// connection
// cookie
// host
// user-agent

// 3) BODY
std::string HttpRequest::getBody() const { return _body; }
std::string HttpRequest::getQueryParameters() const { return _queryParameters; }
// BONUS :
// std::string HttpRequest::getCookies() /* const */ { return _cookies; }

bool HttpRequest::isChunked() const
{
    // TO DO
    return (true);
}

// OTHER
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

// PRINT DATA
std::ostream &operator<<(std::ostream &os, const HttpRequest &req)
{
    os << "Method: " << req.getMethod() << "\n";
    os << "URI: " << req.getURI() << "\n";
    os << "Version: " << req.getHTTPVersion() << "\n";
    os << "Headers: \n";
    std::map<std::string, std::string> headers = req.getHeaders();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
    {
        // traitement
    }
    os << "Body: " << req.getBody() << "\n";
    os << "Query Parameters: " << req.getQueryParameters() << "\n";
    // os << "Cookies: " << req.getCookies() << "\n";
    os << "Chunked: " << (req.isChunked() ? "Yes" : "No") << "\n";
    return os;
}

void HttpRequest::requestController(HttpResponse &response, std::map<std::string, std::string> &resourceDatabase)
{

    // Pointeur de fonction membre pour handle()
    // typedef void (RequestController::*HandlerFunction)(const HttpRequest& req, HttpResponse& res);

    // Création des handlers
    GetRequestHandler getHandler;
    PostRequestHandler postHandler;
    DeleteRequestHandler deleteHandler;
    (void)resourceDatabase;
    // GetRequestHandler getHandler(/* resourceDatabase */);
    // PostRequestHandler postHandler(/* resourceDatabase */);
    // DeleteRequestHandler deleteHandler(/* resourceDatabase */);

    // Map associant les méthodes HTTP avec les handlers
    std::map<std::string, RequestController *> handlerMap;
    handlerMap["GET"] = &getHandler;
    handlerMap["POST"] = &postHandler;
    handlerMap["DELETE"] = &deleteHandler;

    // Trouver le bon handler en fonction de la méthode HTTP
    std::string method = getMethod();
    std::map<std::string, RequestController *>::iterator it = handlerMap.find(method);

    if (it != handlerMap.end())
    {
        RequestController *handler = it->second;
        handler->handle(*this, response);
    }
    else
    {
        response.setStatusCode(405);
        response.setBody("405 Method Not Allowed");
        response.setHeader("Content-Type", "text/plain");
        response.ensureContentLength();
    }
}
