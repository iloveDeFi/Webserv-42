#ifndef HTTPCONTROLLER_HPP
#define HTTPCONTROLLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <fstream>
#include <stdexcept>
#include <string>
#include <map>
#include <set>
#include <unistd.h>   // Pour access()
#include <sys/stat.h> // Pour S_IRUSR

class RequestController
{
private:
    std::map<std::string, std::string> &_resourceDatabase;
    std::set<std::string> _deletionInProgress;
    static const std::set<std::string> _validMethods;

public:
    RequestController(std::map<std::string, std::string> &resourceDatabase);
    RequestController(const RequestController &src);
    RequestController &operator=(const RequestController &src);
    virtual void handle(const HttpRequest &req, HttpResponse &res) = 0;
    virtual ~RequestController();

    std::map<std::string, std::string> &getResourceDatabase();
    bool hasReadPermissions(const std::string &filePath);
    std::string loadResource(const std::string &filePath);
    bool hasPermissionToCreate(const std::string &uri);
    bool hasPermissionToDelete(const std::string &uri) const;
    bool isValidHttpMethod(const std::string &method) const;
    bool isMethodAllowed(const std::string &method) const;

    void handleGetResponse(const HttpRequest &req, HttpResponse &res);
    void handlePostResponse(const HttpRequest &req, HttpResponse &res);
    void handleDeleteResponse(const HttpRequest &req, HttpResponse &res);
    void handleUnknownResponse(const HttpRequest &req, HttpResponse &res);
};

class GetRequestHandler : public RequestController
{
public:
    GetRequestHandler(std::map<std::string, std::string> &resourceDatabase);
    ~GetRequestHandler();
    void handle(const HttpRequest &req, HttpResponse &res);
};

class PostRequestHandler : public RequestController
{
public:
    PostRequestHandler(std::map<std::string, std::string> &resourceDatabase);
    ~PostRequestHandler();
    void handle(const HttpRequest &req, HttpResponse &res);
};

class DeleteRequestHandler : public RequestController
{
public:
    DeleteRequestHandler(std::map<std::string, std::string> &resourceDatabase);
    ~DeleteRequestHandler();
    void handle(const HttpRequest &req, HttpResponse &res);
};

class UnknownRequestHandler : public RequestController
{
public:
    UnknownRequestHandler(std::map<std::string, std::string> &resourceDatabase);
    ~UnknownRequestHandler();
    void handle(const HttpRequest &req, HttpResponse &res);
};

#endif