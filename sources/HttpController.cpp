#include "HttpController.hpp"

const std::set<std::string> RequestController::_validMethods = {
    "GET", "PUT", "DELETE", "UNKNOWN"};

RequestController::RequestController(std::map<std::string, std::string> &resourceDatabase)
    : _resourceDatabase(resourceDatabase), _deletionInProgress()
{
    _resourceDatabase[""] = "";
}

RequestController::RequestController(const RequestController &src)
    : _resourceDatabase(src._resourceDatabase), _deletionInProgress(src._deletionInProgress) {}

RequestController &RequestController::operator=(const RequestController &src)
{
    if (this != &src)
    {
        _resourceDatabase = src._resourceDatabase;
        _deletionInProgress = src._deletionInProgress;
    }
    return *this;
}

RequestController::~RequestController() {}

std::map<std::string, std::string> &RequestController::getResourceDatabase()
{
    return _resourceDatabase;
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
    std::ifstream file(filePath.c_str(), std::ios::in); // Open file in text mode
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
    for (size_t i = 0; i < getServerLocations().size(); ++i)
    {
        const LocationConfig &location = getServerLocations()[i];
        if (uri.find(location.path) == 0)
        {
            if (std::find(location.methods.begin(), location.methods.end(), "DELETE") != location.methods.end())
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}

bool RequestController::isValidHttpMethod(const std::string &method) const
{
    return _validMethods.find(method) != _validMethods.end();
}

bool RequestController::isMethodAllowed(const std::string &method) const
{
    for (size_t i = 0; i < getServerLocations().size(); ++i)
    {
        const LocationConfig &location = getServerLocations()[i];
        if (location.methods.find(method) != location.methods.end())
        {
            return true;
        }
    }
    return false;
}

// ------------------------ GET METHOD --------------------------
void RequestController::handleGetResponse(const HttpRequest &req, HttpResponse &res)
{
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();

    if (version != "HTTP/1.1" && version != "HTTP/1.0")
    {
        res.generate400BadRequest("Invalid HTTP version");
        return;
    }

    std::map<std::string, std::string>::const_iterator resourceIt = getResourceDatabase().find(uri);

    if (resourceIt == getResourceDatabase().end())
    {
        res.generate404NotFound("Resource not found: " + uri);
        return;
    }

    if (!hasReadPermissions(resourceIt->second))
    {
        res.generate403Forbidden("Access to the resource is forbidden");
        return;
    }

    try
    {
        std::string resourceContent = loadResource(resourceIt->second);
        res.generate200OK("text/plain", resourceContent);
    }
    catch (const std::exception &e)
    {
        res.generate500InternalServerError("Internal error 500: An error occurred while processing the request: " + std::string(e.what()));
    }

    res.setHTTPVersion(version);
    res.ensureContentLength();
}

// ------------------------ POST METHOD --------------------------
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

    if (getResourceDatabase().find(uri) != getResourceDatabase().end())
    {
        res.generate409Conflict("409 Conflict: Resource already exists at this URI: " + uri);
        return;
    }

    try
    {
        // Simulate resource creation
        getResourceDatabase()[uri] = body;
        res.generate201Created(uri);
    }
    catch (const std::exception &e)
    {
        res.generate500InternalServerError("500 Internal Server Error: An error occurred while processing the request: " + std::string(e.what()));
    }

    res.setHTTPVersion(version);
    res.ensureContentLength();
}

// ------------------------ DELETE METHOD --------------------------
void RequestController::handleDeleteResponse(const HttpRequest &req, HttpResponse &res)
{
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();

    if (!hasPermissionToDelete(uri))
    {
        res.generate403Forbidden("403 Forbidden: You do not have permission to delete this resource.");
        return;
    }

    if (getResourceDatabase().find(uri) == getResourceDatabase().end())
    {
        res.generate404NotFound("404 Not Found: Resource not found: " + uri);
        return;
    }

    try
    {
        getResourceDatabase().erase(uri);
        res.generate204NoContent("204 No Content : Delete success");
    }
    catch (const std::exception &e)
    {
        res.generate500InternalServerError("500 Internal Server Error: An error occurred while processing the request: " + std::string(e.what()));
    }

    res.setHTTPVersion(version);
}

// ------------------------ UNKNOWN METHOD --------------------------
void RequestController::handleUnknownResponse(const HttpRequest &req, HttpResponse &res)
{
    std::string version = req.getHTTPVersion();
    std::string method = req.getMethod();

    if (method.empty())
    {
        res.generate400BadRequest("400 Bad Request: Method cannot be empty.");
    }
    else if (!isValidHttpMethod(method))
    {
        res.generate400BadRequest("400 Bad Request: Unrecognized or malformed HTTP method.");
    }
    else if (!isMethodAllowed(method))
    {
        res.generate405MethodNotAllowed("405 Method Not Allowed: " + method + " is not allowed.");
    }
    else
    {
        res.generate501NotImplemented("501 Not Implemented: Sorry. Http method not recognized.");
    }

    res.setHTTPVersion(version);
    res.ensureContentLength();
}

// GET
GetRequestHandler::GetRequestHandler(std::map<std::string, std::string> &resourceDatabase)
    : RequestController(resourceDatabase) {}
GetRequestHandler::~GetRequestHandler() {}

void GetRequestHandler::handle(const HttpRequest &req, HttpResponse &res)
{
    handleGetResponse(req, res);
}

// POST
PostRequestHandler::PostRequestHandler(std::map<std::string, std::string> &resourceDatabase)
    : RequestController(resourceDatabase) {}
PostRequestHandler::~PostRequestHandler() {}

void PostRequestHandler::handle(const HttpRequest &req, HttpResponse &res)
{
    handlePostResponse(req, res);
}

// DELETE
DeleteRequestHandler::DeleteRequestHandler(std::map<std::string, std::string> &resourceDatabase)
    : RequestController(resourceDatabase) {}
DeleteRequestHandler::~DeleteRequestHandler() {}

void DeleteRequestHandler::handle(const HttpRequest &req, HttpResponse &res)
{
    handleDeleteResponse(req, res);
}

// UNKNOWN
UnknownRequestHandler::UnknownRequestHandler(std::map<std::string, std::string> &resourceDatabase)
    : RequestController(resourceDatabase) {}
UnknownRequestHandler::~UnknownRequestHandler() {}

void UnknownRequestHandler::handle(const HttpRequest &req, HttpResponse &res)
{
    handleUnknownResponse(req, res);
}