#ifndef HTTPCONTROLLER_HPP
# define HTTPCONTROLLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <string>
#include <map>

class RequestController {
private:
    std::map<std::string, std::string>& _resourceDatabase;

public:
    RequestController();
    RequestController(const RequestController& src);
    RequestController& operator=(const RequestController& src);
    virtual void handle(const HttpRequest& req, HttpResponse& res) = 0;
    virtual ~RequestController();

    std::map<std::string, std::string>& getResourceDatabase() {
        return _resourceDatabase;
    }

    // Méthodes pour gérer les réponses
    void handleGetResponse(const HttpRequest& req, HttpResponse& res);
    void handlePostResponse(const HttpRequest& req, HttpResponse& res);
    void handleDeleteResponse(const HttpRequest& req, HttpResponse& res);
};

class GetRequestHandler : public RequestController {
public:
    GetRequestHandler();
    ~GetRequestHandler();
    void handle(const HttpRequest& req, HttpResponse& res);
};

class PostRequestHandler : public RequestController {
public:
    PostRequestHandler();
    ~PostRequestHandler();
    void handle(const HttpRequest& req, HttpResponse& res);
};

class DeleteRequestHandler : public RequestController {
public:
    DeleteRequestHandler();
    ~DeleteRequestHandler();
    void handle(const HttpRequest& req, HttpResponse& res);
};

#endif