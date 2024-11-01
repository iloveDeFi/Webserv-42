#ifndef HTTPCONTROLLER_HPP
#define HTTPCONTROLLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpConfig.hpp"
#include "Logger.hpp"
#include "Templates.hpp"
#include "MngmtServers.hpp"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <stdexcept>
#include <string>
#include <set>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <algorithm>
#include <iostream>
#include <vector>
//#include <nlohmann/json.hpp>



struct FormData;

class RequestController
{
protected:
    const HttpConfig::Location &_locationConfig;
    std::set<std::string> _deletionInProgress;
    std::set<std::string> _validMethods;
    std::string _serverRoot;
    ServerData _server;

    bool hasReadPermissions(const std::string &filePath);
    std::string loadResource(const std::string &filePath);
    bool hasPermissionToCreate(const std::string &uri);
    bool hasPermissionToDelete(const std::string &uri) const;
    bool isValidHttpMethod(const std::string &method) const;
    bool isMethodAllowed(const std::string &method) const;

    void handleGetResponse(const HttpRequest &req, HttpResponse &res, const ServerData &server);
    void handlePostResponse(const HttpRequest &req, HttpResponse &res, const ServerData &server);
    void handleDeleteResponse(const HttpRequest &req, HttpResponse &res, const ServerData &server);
    void handleOptionsResponse(const HttpRequest &req, HttpResponse &res, const ServerData &server);
    void handleUnknownResponse(const HttpRequest &req, HttpResponse &res, const ServerData &server);
    void handleCgiResponse(const HttpRequest &req, HttpResponse &res, const ServerData &server);

public:
    RequestController(const HttpConfig::Location &locationConfig, const ServerData &server);
    RequestController(const RequestController &src);
    RequestController &operator=(const RequestController &src);
    virtual ~RequestController();

    virtual void handle(const HttpRequest &req, HttpResponse &res, const ServerData &server) = 0;

    // test cors headers
    void setCorsHeaders(HttpResponse &res);
    bool isDirectory(const std::string &path);
    std::string resolveResourcePath(const std::string &uri);
    void serveResource(const std::string &resourcePath, HttpResponse &res, const ServerData &server);
    void handleInternalRequest(const HttpRequest &req, HttpResponse &res, const ServerData &server);
    std::vector<std::string> listFilesInDirectory(const std::string &directoryPath);
    std::string getHandler();

    std::string resolveCgiPath();
    bool isCgiExecutable(const std::string &cgiScriptPath, HttpResponse &res);
    std::vector<std::string> setupEnvironmentVariables(const HttpRequest &req, const std::string &cgiScriptPath);
    bool createPipes(int stdin_pipe[2], int stdout_pipe[2], HttpResponse &res, Logger &logger, const ServerData &server);
    void executeCgiScript(const std::string &cgiScriptPath, const std::vector<std::string> &envVariables, int stdin_pipe[2], int stdout_pipe[2]);
    // std::string communicateWithCgi(int stdin_pipe[2], int stdout_pipe[2], const HttpRequest &req);
    void processCgiOutput(pid_t pid, int stdin_pipe[2], int stdout_pipe[2], const HttpRequest &req, HttpResponse &res, const ServerData &server);
    void handleCgiResponseOutput(const std::string &output, HttpResponse &res);
    void handleError(const std::string &errorMessage, HttpResponse &res, Logger &logger);
    ServerData &getServerInfo();
    bool isPostAllowed(const HttpRequest &req, HttpResponse &res, const ServerData &server);
    void handleMultipartFormData(const HttpRequest &req, HttpResponse &res, const ServerData &server);
    void handleTextData(const HttpRequest &req, HttpResponse &res);
};

class GetRequestHandler : public RequestController
{
public:
    GetRequestHandler(const HttpConfig::Location &locationConfig, const ServerData &server);
    virtual ~GetRequestHandler();
    virtual void handle(const HttpRequest &req, HttpResponse &res, const ServerData &server);
    void handleInternalRequest(const HttpRequest &req, HttpResponse &res, const ServerData &server);
};

class PostRequestHandler : public RequestController
{
public:
    PostRequestHandler(const HttpConfig::Location &locationConfig, const ServerData &server);
    virtual ~PostRequestHandler();
    virtual void handle(const HttpRequest &req, HttpResponse &res, const ServerData &server);
};

class DeleteRequestHandler : public RequestController
{
public:
    DeleteRequestHandler(const HttpConfig::Location &locationConfig, const ServerData &server);
    virtual ~DeleteRequestHandler();
    virtual void handle(const HttpRequest &req, HttpResponse &res, const ServerData &server);
};

class OptionsRequestHandler : public RequestController
{
public:
    OptionsRequestHandler(const HttpConfig::Location &locationConfig, const ServerData &server);
    virtual ~OptionsRequestHandler();
    virtual void handle(const HttpRequest &req, HttpResponse &res, const ServerData &server);
};

class UnknownRequestHandler : public RequestController
{
public:
    UnknownRequestHandler(const HttpConfig::Location &locationConfig, const ServerData &server);
    virtual ~UnknownRequestHandler();
    virtual void handle(const HttpRequest &req, HttpResponse &res, const ServerData &server);
};

class CgiRequestHandler : public RequestController
{
public:
    CgiRequestHandler(const HttpConfig::Location &locationConfig, const ServerData &server);
    virtual ~CgiRequestHandler();
    virtual void handle(const HttpRequest &req, HttpResponse &res, const ServerData &server);
};

#endif
