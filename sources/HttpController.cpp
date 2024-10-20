#include "HttpController.hpp"

RequestController::RequestController(const HttpConfig::Location &locationConfig)
    : _locationConfig(locationConfig), _deletionInProgress()
{
    if (_validMethods.empty())
    {
        _validMethods.insert("GET");
        _validMethods.insert("POST");
        _validMethods.insert("DELETE");
        _validMethods.insert("UNKNOWN");
    }
}

RequestController::RequestController(const RequestController &src)
    : _locationConfig(src._locationConfig), _deletionInProgress(src._deletionInProgress) {}

RequestController &RequestController::operator=(const RequestController &src)
{
    if (this != &src)
    {
        // _locationConfig = src._locationConfig; nope ref constante here
        _deletionInProgress = src._deletionInProgress;
    }
    return *this;
}

RequestController::~RequestController() {}

GetRequestHandler::GetRequestHandler(const HttpConfig::Location &locationConfig)
    : RequestController(locationConfig) {}

GetRequestHandler::~GetRequestHandler() {}

void GetRequestHandler::handle(const HttpRequest &req, HttpResponse &res)
{
    handleGetResponse(req, res);
}

PostRequestHandler::PostRequestHandler(const HttpConfig::Location &locationConfig)
    : RequestController(locationConfig) {}

PostRequestHandler::~PostRequestHandler() {}

void PostRequestHandler::handle(const HttpRequest &req, HttpResponse &res)
{
    handlePostResponse(req, res);
}

DeleteRequestHandler::DeleteRequestHandler(const HttpConfig::Location &locationConfig)
    : RequestController(locationConfig) {}

DeleteRequestHandler::~DeleteRequestHandler() {}

void DeleteRequestHandler::handle(const HttpRequest &req, HttpResponse &res)
{
    handleDeleteResponse(req, res);
}

UnknownRequestHandler::UnknownRequestHandler(const HttpConfig::Location &locationConfig)
    : RequestController(locationConfig) {}

UnknownRequestHandler::~UnknownRequestHandler() {}

void UnknownRequestHandler::handle(const HttpRequest &req, HttpResponse &res)
{
    handleUnknownResponse(req, res);
}

bool RequestController::hasReadPermissions(const std::string &filePath)
{
    if (access(filePath.c_str(), R_OK) == 0)
    {
        struct stat fileStat;
        if (stat(filePath.c_str(), &fileStat) == 0)
        {
            return S_ISREG(fileStat.st_mode);
        }
    }
    return false;
}

std::string RequestController::loadResource(const std::string &filePath)
{
    std::ifstream file(filePath.c_str(), std::ios::in);
    if (!file.is_open())
    {
        throw std::runtime_error("Unable to open file: " + filePath);
    }
    std::string content;
    std::string line;

    while (std::getline(file, line))
    {
        content += line + "\n";
    }

    if (file.bad())
    {
        throw std::runtime_error("Error reading file: " + filePath);
    }

    file.close();
    return content;
}

bool RequestController::hasPermissionToCreate(const std::string &uri)
{
    std::string path = uri;
    size_t lastSlash = path.find_last_of('/');
    std::string parentDir = (lastSlash != std::string::npos) ? path.substr(0, lastSlash) : ".";

    struct stat dirStat;
    if (stat(parentDir.c_str(), &dirStat) != 0)
    {
        return false;
    }

    return (access(parentDir.c_str(), W_OK) == 0);
}

bool RequestController::hasPermissionToDelete(const std::string &uri) const
{
    (void)uri;
    return std::find(_locationConfig.methods.begin(), _locationConfig.methods.end(), "DELETE") != _locationConfig.methods.end();
}

bool RequestController::isValidHttpMethod(const std::string &method) const
{
    return _validMethods.find(method) != _validMethods.end();
}

bool RequestController::isMethodAllowed(const std::string &method) const
{
    return std::find(_locationConfig.methods.begin(), _locationConfig.methods.end(), method) != _locationConfig.methods.end();
}

void RequestController::handleGetResponse(const HttpRequest &req, HttpResponse &res)
{
    Logger &logger = Logger::getInstance("server.log");
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();

    if (version != "HTTP/1.1" && version != "HTTP/1.0")
    {
        res.generate400BadRequest("Invalid HTTP version");
        return;
    }

    std::string resourcePath = _locationConfig.root + uri;

    if (!hasReadPermissions(resourcePath))
    {
        res.generate403Forbidden("Access to the resource is forbidden");
        return;
    }

    try
    {
        std::string resourceContent = loadResource(resourcePath);
        res.generate200OK("text/plain", resourceContent);
        logger.log("Response Status Code: " + to_string(res.getStatusCode()));
        logger.log("Response Body Length: " + to_string(resourceContent.length()));
    }
    catch (const std::exception &e)
    {
        logger.log("Error occurred while loading resource: " + std::string(e.what()));
        res.generate500InternalServerError("Internal error 500: An error occurred while processing the request: " + std::string(e.what()));
    }

    res.setHTTPVersion(version);
    res.ensureContentLength();
}

void RequestController::handlePostResponse(const HttpRequest &req, HttpResponse &res)
{
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();
    std::string body = req.getBody();

    if (body.empty())
    {
        res.generate400BadRequest("400 error : Bad Request: Empty body or malformed request.");
        return;
    }

    if (!hasPermissionToCreate(uri))
    {
        res.generate403Forbidden("Forbidden: You do not have permission to create a resource at this location.");
        return;
    }
    try
    {
        res.generate201Created(uri);
    }
    catch (const std::exception &e)
    {
        res.generate500InternalServerError("500 Internal Server Error: An error occurred while processing the request: " + std::string(e.what()));
    }

    res.setHTTPVersion(version);
    res.ensureContentLength();
}

void RequestController::handleDeleteResponse(const HttpRequest &req, HttpResponse &res)
{
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();

    if (!hasPermissionToDelete(uri))
    {
        res.generate403Forbidden("403 Forbidden: You do not have permission to delete this resource.");
        return;
    }
    std::string resourcePath = _locationConfig.root + uri;
    if (remove(resourcePath.c_str()) != 0)
    {
        res.generate404NotFound("404 Not Found: Resource not found: " + uri);
        return;
    }

    res.generate204NoContent("204 No Content: Delete success");
    res.setHTTPVersion(version);
}

void RequestController::handleUnknownResponse(const HttpRequest &req, HttpResponse &res)
{
    std::string version = req.getHTTPVersion();
    std::string method = req.getMethod();

    if (method.empty())
    {
        res.generate400BadRequest("400 Bad Request: Method cannot be empty.");
        return;
    }

    res.generate405MethodNotAllowed("405 Method Not Allowed: The method " + method + " is not allowed.");
    res.setHTTPVersion(version);
}