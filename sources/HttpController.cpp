#include "HttpController.hpp"
#include <dirent.h>

RequestController::RequestController(const HttpConfig::Location &locationConfig, const std::string &serverRoot)
    : _locationConfig(locationConfig), _deletionInProgress(), _serverRoot(serverRoot)
{
    if (_validMethods.empty())
    {
        _validMethods.insert("GET");
        _validMethods.insert("POST");
        _validMethods.insert("DELETE");
        _validMethods.insert("OPTIONS");
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

GetRequestHandler::GetRequestHandler(const HttpConfig::Location &locationConfig, const std::string &serverRoot)
    : RequestController(locationConfig, serverRoot) {}

GetRequestHandler::~GetRequestHandler() {}

void GetRequestHandler::handle(const HttpRequest &req, HttpResponse &res)
{
    handleGetResponse(req, res);
}

PostRequestHandler::PostRequestHandler(const HttpConfig::Location &locationConfig, const std::string &serverRoot)
    : RequestController(locationConfig, serverRoot) {}

PostRequestHandler::~PostRequestHandler() {}

void PostRequestHandler::handle(const HttpRequest &req, HttpResponse &res)
{
    handlePostResponse(req, res);
}

DeleteRequestHandler::DeleteRequestHandler(const HttpConfig::Location &locationConfig, const std::string &serverRoot)
    : RequestController(locationConfig, serverRoot) {}

DeleteRequestHandler::~DeleteRequestHandler() {}

void DeleteRequestHandler::handle(const HttpRequest &req, HttpResponse &res)
{
    handleDeleteResponse(req, res);
}

OptionsRequestHandler::OptionsRequestHandler(const HttpConfig::Location &locationConfig, const std::string &serverRoot)
    : RequestController(locationConfig, serverRoot) {}

OptionsRequestHandler::~OptionsRequestHandler() {}

void OptionsRequestHandler::handle(const HttpRequest &req, HttpResponse &res)
{
    handleOptionsResponse(req, res);
}

UnknownRequestHandler::UnknownRequestHandler(const HttpConfig::Location &locationConfig, const std::string &serverRoot)
    : RequestController(locationConfig, serverRoot) {}

UnknownRequestHandler::~UnknownRequestHandler() {}

void UnknownRequestHandler::handle(const HttpRequest &req, HttpResponse &res)
{
    handleUnknownResponse(req, res);
}

bool RequestController::hasReadPermissions(const std::string &filePath)
{
    Logger &logger = Logger::getInstance("server.log");
    logger.log("Checking read permissions for: " + filePath);

    if (access(filePath.c_str(), R_OK) == 0)
    {
        struct stat fileStat;
        if (stat(filePath.c_str(), &fileStat) == 0)
        {
            logger.log("File exists and is accessible.");
            return S_ISREG(fileStat.st_mode);
        }
    }
    logger.log("Access denied or file does not exist.");
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
    (void)uri; // Not needed
    // Check if uploads are allowed in the location configuration
    if (!_locationConfig.allowUploads)
    {
        return false;
    }
    // Check if the uploads directory exists and is writable
    std::string uploadsDir = _serverRoot + "/index/files/";
    struct stat dirStat;
    if (stat(uploadsDir.c_str(), &dirStat) != 0 || !S_ISDIR(dirStat.st_mode))
    {
        return false;
    }
    return (access(uploadsDir.c_str(), W_OK) == 0);
}


bool RequestController::hasPermissionToDelete(const std::string &uri) const
{
    (void)uri;
    // Check if DELETE is allowed for this location
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

bool RequestController::isDirectory(const std::string &path)
{
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0)
    {
        return false;
    }
    return S_ISDIR(statbuf.st_mode);
}

std::string RequestController::resolveResourcePath(const std::string &uri)
{
    std::string resourcePath = _serverRoot;
    if (resourcePath[resourcePath.length() - 1] != '/')
        resourcePath += '/';

    if (!_locationConfig.handler.empty())
    {
         resourcePath += _locationConfig.handler;
    }
    else
    {
        // Default behavior if no handler is specified
        std::string finalUri = uri;
        if (finalUri[0] == '/')
            finalUri = finalUri.substr(1);

        resourcePath += finalUri;

        if (isDirectory(resourcePath))
        {
            if (resourcePath.back() != '/')
                resourcePath += '/';
            resourcePath += "index.html";
        }
    }

    return resourcePath;
}



void RequestController::serveResource(const std::string &resourcePath, HttpResponse &res)
{
    Logger &logger = Logger::getInstance("server.log");

    try
    {
        std::string resourceContent = loadResource(resourcePath);
        res.generate200OK("text/html", resourceContent);
        logger.log("Response Status Code: " + to_string(res.getStatusCode()));
        logger.log("Response Body Length: " + to_string(resourceContent.length()));
        setCorsHeaders(res);
    }
    catch (const std::exception &e)
    {
        logger.log("Error occurred while loading resource: " + std::string(e.what()));
        res.generate500InternalServerError("Internal error 500: " + std::string(e.what()));
    }

    res.ensureContentLength();
    res.logHttpResponse(logger);
}

void RequestController::handleGetResponse(const HttpRequest &req, HttpResponse &res)
{
    Logger &logger = Logger::getInstance("server.log");
    std::string uri = req.getURI();
    logger.log("Received URI: " + uri);

    std::string version = req.getHTTPVersion();
    if (version != "HTTP/1.1" && version != "HTTP/1.0")
    {
        res.generate400BadRequest("Invalid HTTP version");
        logger.log("Invalid HTTP version received: " + version);
        return;
    }

    // Check if handler is "internal"
    if (_locationConfig.handler == "internal")
    {
        handleInternalRequest(req, res);
        return;
    }

    std::string resourcePath = resolveResourcePath(uri);
    logger.log("Resolved resource path: " + resourcePath);

    if (!hasReadPermissions(resourcePath))
    {
        res.generate403Forbidden("403 Forbidden: Access to the resource is forbidden");
        logger.log("Error: Access to the resource is forbidden for resourcePath: " + resourcePath);
        return;
    }

    serveResource(resourcePath, res);
}

void RequestController::handlePostResponse(const HttpRequest &req, HttpResponse &res)
{
    Logger &logger = Logger::getInstance("server.log");
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();
    std::string body = req.getBody();
    logger.log("Received POST request for URI: " + uri);

    if (body.empty())
    {
        res.generate400BadRequest("Bad Request: Empty body or malformed request.");
        return;
    }

    // Check if POST method is allowed for this location
    if (!isMethodAllowed("POST"))
    {
        res.generate405MethodNotAllowed("POST method not allowed for this location.");
        return;
    }

    // Check if uploads are allowed in this location
    if (!_locationConfig.allowUploads)
    {
        res.generate403Forbidden("Forbidden: Uploads are not allowed at this location.");
        return;
    }

    // Check if the uploads directory exists and is writable
    std::string uploadsDir = _serverRoot + "/uploads/";
    struct stat dirStat;
    if (stat(uploadsDir.c_str(), &dirStat) != 0 || !S_ISDIR(dirStat.st_mode) || access(uploadsDir.c_str(), W_OK) != 0)
    {
        res.generate500InternalServerError("Uploads directory does not exist or is not writable.");
        return;
    }

    try
    {
        // Parse the multipart/form-data
        std::string boundary = req.getBoundary();
        if (boundary.empty())
        {
            throw std::runtime_error("No boundary found in Content-Type header");
        }

        HttpRequest::FormData formData = req.parseMultipartFormData();

        // Assuming the file field is named "file"
        if (formData.fields.find("file") == formData.fields.end())
        {
            throw std::runtime_error("No file found in form data");
        }

        std::string fileContent = formData.fields["file"];
        std::string fileName = formData.fileName;

        // Security check to prevent directory traversal attacks
        if (fileName.find("..") != std::string::npos)
        {
            res.generate403Forbidden("Forbidden: Invalid filename");
            return;
        }

        // Check if the file already exists
        std::string filePath = uploadsDir + fileName;
        std::ifstream existingFile(filePath.c_str());
        if (existingFile.good())
        {
            existingFile.close(); // Close the file if it exists
            res.generate409Conflict("Conflict: The file already exists.");
            logger.log("409 Conflict: File already exists: " + filePath);
            return; // Early return to avoid writing the file
        }
        existingFile.close(); // Close the file if it was opened

        // Save the file to the uploads directory
        std::ofstream outFile(filePath.c_str(), std::ios::binary);
        if (!outFile.is_open())
        {
            throw std::runtime_error("Failed to open file for writing: " + filePath);
        }
        outFile.write(fileContent.c_str(), fileContent.size());
        outFile.close();

        res.generate201Created("/files/" + fileName);
        logger.log("File uploaded successfully: " + filePath);
    }
    catch (const std::exception &e)
    {
        res.generate500InternalServerError("Internal Server Error: " + std::string(e.what()));
        logger.log("Error processing POST request: " + std::string(e.what()));
    }

    res.setHTTPVersion(version);
    res.ensureContentLength();
    setCorsHeaders(res);
}



void RequestController::handleDeleteResponse(const HttpRequest &req, HttpResponse &res)
{
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();
    Logger &logger = Logger::getInstance("server.log");
    logger.log("Received DELETE request for URI: " + uri);
    
    if (!hasPermissionToDelete(uri))
    {
        res.generate403Forbidden("403 Forbidden: You do not have permission to delete this resource.");
        return;
    }

    // Expected format: /files/filename
    const std::string prefix = "/files/";
    if (uri.compare(0, prefix.length(), prefix) != 0)
    {
        res.generate404NotFound("Invalid URI for DELETE operation: " + uri);
        return;
    }

    std::string filename = uri.substr(prefix.length());

    // Security check to prevent directory traversal attacks
    if (filename.find("..") != std::string::npos)
    {
        res.generate403Forbidden("Forbidden: Invalid filename");
        return;
    }

    // Construct the full path to the file in the files directory
    std::string resourcePath = _serverRoot + "/index/files/" + filename;

    // Attempt to delete the file
    if (remove(resourcePath.c_str()) != 0)
    {
        res.generate404NotFound("404 Not Found: Resource not found: " + uri);
        return;
    }
    logger.log("File deleted successfully: " + resourcePath);
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

    res.generate501NotImplemented("501 Not Implemented: The server does not recognize the HTTP method used: " + method);
    res.setHTTPVersion(version);
}

void RequestController::setCorsHeaders(HttpResponse &res)
{
    res.setHeader("Access-Control-Allow-Origin", "*");                  // Permet toutes les origines
    res.setHeader("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS"); // Méthodes autorisées
    res.setHeader("Access-Control-Allow-Headers", "Content-Type");      // En-têtes autorisés
}


void RequestController::handleInternalRequest(const HttpRequest &req, HttpResponse &res)
{
    Logger &logger = Logger::getInstance("server.log");
    std::string uri = req.getURI();
    logger.log("Handling internal request for URI: " + uri);

    if (uri == "/file-list")
    {
        std::string filesDir = _serverRoot + "/index/files";
        std::vector<std::string> files = listFilesInDirectory(filesDir);

        // Generate JSON response
        std::string jsonResponse = "{\"files\":[";
        for (size_t i = 0; i < files.size(); ++i)
        {
            jsonResponse += "\"" + files[i] + "\"";
            if (i < files.size() - 1)
                jsonResponse += ",";
        }
        jsonResponse += "]}";

        res.setStatusCode(200);
        res.setReasonMessage("OK");
        res.setHeader("Content-Type", "application/json");
        res.setBody(jsonResponse);
        res.ensureContentLength();
    }
    else
    {
        res.generate404NotFound("Invalid internal URI: " + uri);
        logger.log("Invalid internal URI: " + uri);
    }
}


std::vector<std::string> RequestController::listFilesInDirectory(const std::string &directoryPath)
{
    std::vector<std::string> files;
    DIR *dir = opendir(directoryPath.c_str());
    if (dir == NULL)
    {
        Logger &logger = Logger::getInstance("server.log");
        logger.log("Unable to open directory: " + directoryPath);
        return files;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string fileName = entry->d_name;
        if (fileName != "." && fileName != "..")
        {
            files.push_back(fileName);
        }
    }
    closedir(dir);
    return files;
}


void RequestController::handleOptionsResponse(const HttpRequest &req, HttpResponse &res)
{
    (void)req;
    res.setStatusCode(204);  // No Content
    res.setReasonMessage("No Content");
    res.setHeader("Access-Control-Allow-Origin", "*");
    res.setHeader("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
    res.setHeader("Access-Control-Allow-Headers", "Content-Type");
    res.setHeader("Access-Control-Max-Age", "86400"); 
    res.setBody("");
}




