#include "HttpController.hpp"

void GetRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();
    std::string queryParams = req.getQueryParameters();
    std::string cookies = req.getCookies();

    std::string responseBody = "You requested: " + uri + "\nQuery Params: " + queryParams + "\nCookies: " + cookies;

    res.setHTTPVersion(version);
    res.setStatusCode(200);
    res.setHeader("Content-Type", "text/plain");
    res.setBody(responseBody);
    res.ensureContentLength();
}

void PostRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();
    std::string body = req.getBody();
    std::string cookies = req.getCookies();
    std::string queryParams = req.getQueryParameters(); 

    std::string responseBody = "Received POST data for: " + uri + "\nBody: " + body + "\nCookies: " + cookies;

    res.setStatusCode(201);
    res.setBody(responseBody);
    res.setHTTPVersion(version);
    res.setHeader("Content-Type", "text/plain");
    res.ensureContentLength();
}

bool checkResourceExists(const std::string& uri, const std::map<std::string, std::string>& resourceDatabase) {
    return resourceDatabase.find(uri) != resourceDatabase.end();
}

bool deleteResource(const std::string& uri, std::map<std::string, std::string>& resourceDatabase) {
    std::map<std::string, std::string>::iterator it = resourceDatabase.find(uri);
    if (it != resourceDatabase.end()) {
        resourceDatabase.erase(it);
        return true;
    }
    return false;
}

void DeleteRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();

    bool resourceExists = checkResourceExists(uri, _resourceDatabase);
    std::string responseBody;

    if (resourceExists) {
        deleteResource(uri, _resourceDatabase);
        responseBody = "Resource deleted: " + uri;
        res.setStatusCode(200);
    } else {
        responseBody = "Resource not found: " + uri;
        res.setStatusCode(404);
    }

    res.setBody(responseBody);
    res.setHTTPVersion(version);
    res.setHeader("Content-Type", "text/plain");
    res.ensureContentLength();
}

void HttpRequest::requestController(HttpResponse& response, std::map<std::string, std::string>& resourceDatabase) {
    // Map to associate methods with handlers
    typedef void (RequestController::*HandlerFunction)(const HttpRequest&, HttpResponse&);

    std::map<std::string, HandlerFunction> handlerMap;

    GetRequestHandler getHandler;
    PostRequestHandler postHandler;
    DeleteRequestHandler deleteHandler(resourceDatabase);

    handlerMap["GET"] = &GetRequestHandler::handle;
    handlerMap["POST"] = &PostRequestHandler::handle;
    handlerMap["DELETE"] = &DeleteRequestHandler::handle;

    std::map<std::string, HandlerFunction>::iterator it = handlerMap.find(getMethod());
    if (it != handlerMap.end()) {
        (this->*(it->second))(response);
    } else {
        response.setStatusCode(405);
        response.setBody("405 Method Not Allowed");
        response.setHeader("Content-Type", "text/plain");
        response.ensureContentLength();
    }
}
