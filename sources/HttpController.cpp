#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpController.hpp"

// MY PATTERN TO HANDLE HTTP REQUEST AND BUILD HTTP RESPONSE
// 1) Get infos with req object (req.<method>)
// 2) Handle data 
// 3) Set response with res object (res.<method>)

void GetRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();
    std::string queryParams = req.getQueryParameters();
    std::string headers = req.getHeadersAsString();
    std::string cookies = req.getCookies();

    std::string responseBody = "You requested: " + uri + "\nQuery Params: " + queryParams + "\nHeaders: " + headers + "\nCookies: " + cookies;

    res.setStatusCode(200);
    res.setStatusMessage("OK");
    res.setBody(responseBody);
    res.setHTTPVersion(version);
    res.addHeader("Content-Type", "text/plain");
    res.addHeader("Content-Length", std::to_string(responseBody.size()));
}


void PostRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();
    std::string body = req.getBody();
    std::string headers = req.getHeadersAsString(); 
    std::string cookies = req.getCookies(); 
    std::string queryParams = req.getQueryParameters(); 
    
    std::string responseBody = "Received POST data for: " + uri + "\nBody: " + body + "\nHeaders: " + headers + "\nCookies: " + cookies;

    res.setStatusCode(201);
    res.setStatusMessage("Created");
    res.setBody(responseBody);
    res.setHTTPVersion(version);
    res.addHeader("Content-Type", "text/plain");
    res.addHeader("Content-Length", std::to_string(responseBody.size()));
}


void DeleteRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();
    std::string headers = req.getHeadersAsString(); 
    std::string cookies = req.getCookies(); 
    std::string queryParams = req.getQueryParameters(); 

    bool resourceExists = checkResourceExists(uri);
    std::string responseBody;
    if (resourceExists) {
        deleteResource(uri);
        responseBody = "Resource deleted: " + uri;
        res.setStatusCode(200);
        res.setStatusMessage("OK");
    } else {
        responseBody = "Resource not found: " + uri;
        res.setStatusCode(404);
        res.setStatusMessage("Not Found");
    }

    res.setBody(responseBody);
    res.setHTTPVersion(version);
    res.addHeader("Content-Type", "text/plain");
    res.addHeader("Content-Length", std::to_string(responseBody.size()));
}

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