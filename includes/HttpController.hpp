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
protected:
    std::map<std::string, std::string> &_resourceDatabase;
    std::set<std::string> _deletionInProgress;
    static const std::set<std::string> _validMethods;

    bool hasReadPermissions(const std::string &filePath);
    std::string loadResource(const std::string &filePath);
    bool hasPermissionToCreate(const std::string &uri);
    bool hasPermissionToDelete(const std::string &uri) const;
    bool isValidHttpMethod(const std::string &method) const;
    bool isMethodAllowed(const std::string &method) const;

public:
    RequestController(std::map<std::string, std::string> &resourceDatabase);
    RequestController(const RequestController &src);
    RequestController &operator=(const RequestController &src);
    virtual ~RequestController();

    virtual void handle(const HttpRequest &req, HttpResponse &res) = 0;
    std::map<std::string, std::string> &getResourceDatabase();
};
class GetRequestHandler : public RequestController
{
public:
    GetRequestHandler(std::map<std::string, std::string> &resourceDatabase);
    virtual ~GetRequestHandler();
    virtual void handle(const HttpRequest &req, HttpResponse &res);
};

class PostRequestHandler : public RequestController
{
public:
    PostRequestHandler(std::map<std::string, std::string> &resourceDatabase);
    virtual ~PostRequestHandler();
    virtual void handle(const HttpRequest &req, HttpResponse &res);
};

class DeleteRequestHandler : public RequestController
{
public:
    DeleteRequestHandler(std::map<std::string, std::string> &resourceDatabase);
    virtual ~DeleteRequestHandler();
    virtual void handle(const HttpRequest &req, HttpResponse &res);
};

class UnknownRequestHandler : public RequestController
{
public:
    UnknownRequestHandler(std::map<std::string, std::string> &resourceDatabase);
    virtual ~UnknownRequestHandler();
    virtual void handle(const HttpRequest &req, HttpResponse &res);
};

// Implémentation des constructeurs
GetRequestHandler::GetRequestHandler(std::map<std::string, std::string> &resourceDatabase)
    : RequestController(resourceDatabase) {}

GetRequestHandler::~GetRequestHandler() {}

void GetRequestHandler::handle(const HttpRequest &req, HttpResponse &res)
{
    // Logique pour gérer la requête GET
}

PostRequestHandler::PostRequestHandler(std::map<std::string, std::string> &resourceDatabase)
    : RequestController(resourceDatabase) {}

PostRequestHandler::~PostRequestHandler() {}

void PostRequestHandler::handle(const HttpRequest &req, HttpResponse &res)
{
    // Logique pour gérer la requête POST
}

DeleteRequestHandler::DeleteRequestHandler(std::map<std::string, std::string> &resourceDatabase)
    : RequestController(resourceDatabase) {}

DeleteRequestHandler::~DeleteRequestHandler() {}

void DeleteRequestHandler::handle(const HttpRequest &req, HttpResponse &res)
{
    // Logique pour gérer la requête DELETE
}

UnknownRequestHandler::UnknownRequestHandler(std::map<std::string, std::string> &resourceDatabase)
    : RequestController(resourceDatabase) {}

UnknownRequestHandler::~UnknownRequestHandler() {}

void UnknownRequestHandler::handle(const HttpRequest &req, HttpResponse &res)
{
    // Logique pour gérer la requête UNKNOWN
}

#endif