#include "HttpRequest.hpp"

// TEMPLATE FOR HTTP 
// --- 1) Request message general format:
// HTTP Request Line : METHOD_VERB/CHEMIN_RESSOURCE/PROTOCOL_HTPP
// HTTP Request HeaderS : NAME OR VALUE
// (empty line)
// HTTP Request Body

// --- 2) Response message general format:
// HTTP Response Line
// HTTP Response Headers
// (empty line)
// HTTP Response Body


void GetRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    // get my request infos
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();
    std::string queryParams = req.getQueryParameters(); // Si implémenté
    std::string headers = req.getHeadersAsString(); // Simplification possible pour afficher les headers
    std::string cookies = req.getCookies(); // Si géré par une méthode

    // Exemple de contenu pour la réponse
    std::string responseBody = "You requested: " + uri + "\nQuery Params: " + queryParams + "\nHeaders: " + headers + "\nCookies: " + cookies;

    // set my response
    res.setStatusCode(200); // HTTP 200 OK
    res.setStatusMessage("OK");
    res.setBody(responseBody);
    res.setHTTPVersion(version);
    // additional headers
    res.addHeader("Content-Type", "text/plain");
    res.addHeader("Content-Length", std::to_string(responseBody.size()));
}


void PostRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    // get my request infos
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();
    std::string body = req.getBody(); // Corps de la requête POST
    std::string headers = req.getHeadersAsString(); // Simplification pour afficher les headers
    std::string cookies = req.getCookies(); // Si géré
    std::string queryParams = req.getQueryParameters(); // Si géré
    
    // Gérer la logique spécifique (e.g., stockage de données, création de ressources)
    std::string responseBody = "Received POST data for: " + uri + "\nBody: " + body + "\nHeaders: " + headers + "\nCookies: " + cookies;

    // set my response
    res.setStatusCode(201); // HTTP 201 Created
    res.setStatusMessage("Created");
    res.setBody(responseBody);
    res.setHTTPVersion(version);
    // additional headers
    res.addHeader("Content-Type", "text/plain");
    res.addHeader("Content-Length", std::to_string(responseBody.size()));
}


void DeleteRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    // get my request infos
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();
    std::string headers = req.getHeadersAsString(); // Simplification pour afficher les headers
    std::string cookies = req.getCookies(); // Si géré
    std::string queryParams = req.getQueryParameters(); // Si géré

    // If resource exist delete
    bool resourceExists = checkResourceExists(uri); // fonction pour l'existence de la ressource
    std::string responseBody;
    if (resourceExists) {
        deleteResource(uri); // fonction pour suppression de la ressource
        responseBody = "Resource deleted: " + uri;
        res.setStatusCode(200); // HTTP 200 OK
        res.setStatusMessage("OK");
    } else {
        responseBody = "Resource not found: " + uri;
        res.setStatusCode(404); // HTTP 404 Not Found
        res.setStatusMessage("Not Found");
    }

    // set my response
    res.setBody(responseBody);
    res.setHTTPVersion(version);
    // additonal headers
    res.addHeader("Content-Type", "text/plain");
    res.addHeader("Content-Length", std::to_string(responseBody.size()));
}

// ----- HTTP REQUEST FUNCTIONS ----- :
HttpRequest::HttpRequest() : _method(""), _uri(""), _version("HTTP/1.1"), _allowedMethods(initMethods()) {}

HttpRequest::~HttpRequest() {}

HttpRequest::HttpRequest(const HttpRequest& src) : _method(src._method), _uri(src._uri), _version(src._version), _headers(src._headers), _body(src._body) {}

HttpRequest& HttpRequest::operator=(const HttpRequest& src) {
    if (this != &src) {
        this->_method = src._method;
        this->_uri = src._uri;
        this->_version = src._version;
        this->_headersAsString = src._headersAsString;
        this->_body = src._body;
        this->_is_chunked = src._is_chunked;
    }
    return *this;
}

// ----- GETTERS ----- :
// 1) REQUEST LINE
std::string HttpRequest::getMethod() const { return _method; }
std::string HttpRequest::getURI() const { return _uri; }
std::string HttpRequest::getHTTPVersion() const { return _version; }
// 2) HEADERS
std::map<std::string, std::string> HttpRequest::getHeaders() const { return _headersAsString; }
std::string HttpRequest::getHeader(const std::string& name) const {
    std::map<std::string, std::string>::const_iterator it = _headersAsString.find(name);
        if (it != _headersAsString.end()) {
        return it->second;
    } else {
        return "";
    }
}
// 3) BODY
std::string HttpRequest::getBody() const { return _body; }
std::string HttpRequest::getQueryParameters() const { return _queryParameters; }
std::string HttpRequest::getCookies() const { return _cookies; }
bool HttpRequest::isChunked() const { return _is_chunked; }

// ----- SETTERS ----- :
// 1) RESPONSE LINE
void HttpRequest::setMethod(const std::string& method) { _method = method; }
void HttpRequest::setURI(const std::string& uri) { _uri = uri; }
void HttpRequest::setHTTPVersion(const std::string& version) { _version = version; }
// 2) HEADERS
void HttpRequest::setHeaders(const std::map<std::string, std::string>& headers) { _headers = headers; }
// 3) BODY
void HttpRequest::setBody(const std::string& body) { _body = body; }
void HttpRequest::setIsChunked(bool is_chunked) { _is_chunked = is_chunked; }


// Cleaner to pass from std::vector with linear search O(n) to std::set O(log n) 
// with loarithmic search and also ensure element unicity
std::set<std::string> HttpRequest::initMethods() const {
    std::set<std::string> methods;

    methods.insert("GET");
    methods.insert("POST");
    methods.insert("DELETE");
    // methods.insert("PUT");
    // methods.insert("HEAD");
    // methods.insert("CONNECT");
    // methods.insert("OPTIONS");
    // methods.insert("TRACE");
    // methods.insert("PATCH");
    // methods.insert("UNSUPPORTED");
    
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

// ----- REQUEST CONTROLLER CLASS ----- :
void HttpRequest::RequestController(HttpResponse& response) {
    // Map pour associer les méthodes aux gestionnaires
    typedef void (RequestController::*HandlerFunction)(const HttpRequest&, HttpResponse&);

    std::map<std::string, HandlerFunction> handlerMap;
    GetRequestHandler getHandler;
    PostRequestHandler postHandler;
    DeleteRequestHandler deleteHandler;
    // PutRequestHandler putHandler;
    // HeadRequestHandler headHandler;
    // ConnectRequestHandler connectHandler;
    // OptionsRequestHandler optionsHandler;
    // TraceRequestHandler traceHandler;
    // PatchRequestHandler patchHandler;
    // UnsupportedRequestHandler unsupportedHandler;

    handlerMap["GET"] = &GetRequestHandler::handle;
    handlerMap["POST"] = &PostRequestHandler::handle;
    handlerMap["DELETE"] = &DeleteRequestHandler::handle;
    // handlerMap["PUT"] = &PutRequestHandler::handle;
    // handlerMap["HEAD"] = &HeadRequestHandler::handle;
    // handlerMap["CONNECT"] = &ConnectRequestHandler::handle;
    // handlerMap["OPTIONS"] = &OptionsRequestHandler::handle;
    // handlerMap["TRACE"] = &TraceRequestHandler::handle;
    // handlerMap["PATCH"] = &PatchRequestHandler::handle;
    // handlerMap["UNSUPPORTED"] = &UnsupportedRequestHandler::handle;

    std::map<std::string, HandlerFunction>::iterator it = handlerMap.find(getMethod());
    if (it != handlerMap.end()) {
        (it->second)(*this, response);
    } else {
        unsupportedHandler.handle(*this, response);
    }
}