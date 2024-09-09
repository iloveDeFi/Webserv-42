#include "HttpRequest.hpp"

void GetRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    // Récupérer les informations de la requête
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();
    
    // Exemple de contenu pour la réponse
    std::string responseBody = "You requested: " + uri;

    // Définir les détails de la réponse
    res.setStatusCode(200); // HTTP 200 OK
    res.setStatusMessage("OK");
    res.setBody(responseBody);
    res.setHTTPVersion(version);
}

void HeadRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    
}

void PostRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    
}

void PutRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    
}

void DeleteRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    
}

void OptionsRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    
}

void GetRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    
}

void PatchRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    
}

void UnsupportedRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    
}


HttpRequest::HttpRequest() : _method(""), _uri(""), _version("HTTP/1.1"), _allowedMethods(initMethods()) {}

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
    std::map<std::string, std::string>::const_iterator it = _headers.find(name);
        if (it != _headers.end()) {
        return it->second;
    } else {
        return "";
    }
}

// SETTERS
void HttpRequest::setMethod(const std::string& method) { _method = method; }
void HttpRequest::setURI(const std::string& uri) { _uri = uri; }
void HttpRequest::setHTTPVersion(const std::string& version) { _version = version; }
void HttpRequest::setHeaders(const std::map<std::string, std::string>& headers) { _headers = headers; }
void HttpRequest::setBody(const std::string& body) { _body = body; }
void HttpRequest::setIsChunked(bool is_chunked) { _is_chunked = is_chunked; }


// Cleaner to pass from std::vector with linear search O(n) to std::set O(log n) 
// with loarithmic search and also for element unicity
std::set<std::string> HttpRequest::initMethods() const {
    std::set<std::string> methods;

    methods.insert("GET");
    methods.insert("HEAD");
    methods.insert("POST");
    methods.insert("PUT");
    methods.insert("DELETE");
    methods.insert("CONNECT");
    methods.insert("OPTIONS");
    methods.insert("TRACE");
    methods.insert("PATCH");
    methods.insert("UNSUPPORTED");
    
    return methods;
}


bool HttpRequest::isMethodAllowed(const std::string& method) const {
    std::set<std::string>::const_iterator it = _allowedMethods.find(method);
    return it != _allowedMethods.end();
}

// OR
// bool HttpRequest::isMethodAllowed(const std::string& method) const {
//     return initMethods().count(method) > 0;
// }


void HttpRequest::requestController(HttpResponse& response) {
    // Map pour associer les méthodes aux gestionnaires
    typedef void (RequestController::*HandlerFunction)(const HttpRequest&, HttpResponse&);

    std::map<std::string, HandlerFunction> handlerMap;
    GetRequestHandler getHandler;
    HeadRequestHandler headHandler;
    PostRequestHandler postHandler;
    PutRequestHandler putHandler;
    DeleteRequestHandler deleteHandler;
    ConnectRequestHandler connectHandler;
    OptionsRequestHandler optionsHandler;
    TraceRequestHandler traceHandler;
    PatchRequestHandler patchHandler;
    UnsupportedRequestHandler unsupportedHandler;

    handlerMap["GET"] = &GetRequestHandler::handle;
    handlerMap["HEAD"] = &HeadRequestHandler::handle;
    handlerMap["POST"] = &PostRequestHandler::handle;
    handlerMap["PUT"] = &PutRequestHandler::handle;
    handlerMap["DELETE"] = &DeleteRequestHandler::handle;
    handlerMap["CONNECT"] = &ConnectRequestHandler::handle;
    handlerMap["OPTIONS"] = &OptionsRequestHandler::handle;
    handlerMap["TRACE"] = &TraceRequestHandler::handle;
    handlerMap["PATCH"] = &PatchRequestHandler::handle;
    handlerMap["UNSUPPORTED"] = &UnsupportedRequestHandler::handle;

    std::map<std::string, HandlerFunction>::iterator it = handlerMap.find(getMethod());
    if (it != handlerMap.end()) {
        (it->second)(*this, response);  // Appeler la fonction correspondante
    } else {
        unsupportedHandler.handle(*this, response);
    }
}