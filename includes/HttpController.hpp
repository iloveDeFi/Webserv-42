#ifndef HTTPCONTROLLER_HPP
#define HTTPCONTROLLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpConfig.hpp"
#include "Logger.hpp"
#include "Templates.hpp"
#include <fstream>
#include <stdexcept>
#include <string>
#include <set>
#include <unistd.h>
#include <sys/stat.h>
#include <algorithm>

class RequestController
{
protected:
    const HttpConfig::Location &_locationConfig;
    std::set<std::string> _deletionInProgress;
    std::set<std::string> _validMethods;

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

public:
    RequestController(const HttpConfig::Location &locationConfig);
    RequestController(const RequestController &src);
    RequestController &operator=(const RequestController &src);
    virtual ~RequestController();

    virtual void handle(const HttpRequest &req, HttpResponse &res) = 0;

    // test cors headers
    void setCorsHeaders(HttpResponse &res);
};

class GetRequestHandler : public RequestController
{
public:
    GetRequestHandler(const HttpConfig::Location &locationConfig);
    virtual ~GetRequestHandler();
    virtual void handle(const HttpRequest &req, HttpResponse &res);
};

class PostRequestHandler : public RequestController
{
public:
    PostRequestHandler(const HttpConfig::Location &locationConfig);
    virtual ~PostRequestHandler();
    virtual void handle(const HttpRequest &req, HttpResponse &res);
};

class DeleteRequestHandler : public RequestController
{
public:
    DeleteRequestHandler(const HttpConfig::Location &locationConfig);
    virtual ~DeleteRequestHandler();
    virtual void handle(const HttpRequest &req, HttpResponse &res);
};

class UnknownRequestHandler : public RequestController
{
public:
    UnknownRequestHandler(const HttpConfig::Location &locationConfig);
    virtual ~UnknownRequestHandler();
    virtual void handle(const HttpRequest &req, HttpResponse &res);
};

#endif