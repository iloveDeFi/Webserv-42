#include "HttpController.hpp"
#include <dirent.h>

RequestController::RequestController(const HttpConfig::Location &locationConfig, const ServerData &server)
    : _locationConfig(locationConfig), _deletionInProgress(), _serverRoot(server._root), _server(server)
{
    if (_validMethods.empty())
    {
        _validMethods.insert("GET");
        _validMethods.insert("POST");
        _validMethods.insert("DELETE");
        _validMethods.insert("OPTIONS");
        _validMethods.insert("UNKNOWN");
        _validMethods.insert("CGI");
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

GetRequestHandler::GetRequestHandler(const HttpConfig::Location &locationConfig, const ServerData &server)
    : RequestController(locationConfig, server) {}

GetRequestHandler::~GetRequestHandler() {}

void GetRequestHandler::handle(const HttpRequest &req, HttpResponse &res, const ServerData &server)
{
    handleGetResponse(req, res, server);
}

PostRequestHandler::PostRequestHandler(const HttpConfig::Location &locationConfig, const ServerData &server)
    : RequestController(locationConfig, server) {}

PostRequestHandler::~PostRequestHandler() {}

void PostRequestHandler::handle(const HttpRequest &req, HttpResponse &res, const ServerData &server)
{
    handlePostResponse(req, res, server);
}

DeleteRequestHandler::DeleteRequestHandler(const HttpConfig::Location &locationConfig, const ServerData &server)
    : RequestController(locationConfig, server) {}

DeleteRequestHandler::~DeleteRequestHandler() {}

void DeleteRequestHandler::handle(const HttpRequest &req, HttpResponse &res, const ServerData &server)
{
    handleDeleteResponse(req, res, server);
}

OptionsRequestHandler::OptionsRequestHandler(const HttpConfig::Location &locationConfig, const ServerData &server)
    : RequestController(locationConfig, server) {}

OptionsRequestHandler::~OptionsRequestHandler() {}

void OptionsRequestHandler::handle(const HttpRequest &req, HttpResponse &res, const ServerData &server)
{
    handleOptionsResponse(req, res, server);
}

UnknownRequestHandler::UnknownRequestHandler(const HttpConfig::Location &locationConfig, const ServerData &server)
    : RequestController(locationConfig, server) {}

UnknownRequestHandler::~UnknownRequestHandler() {}

void UnknownRequestHandler::handle(const HttpRequest &req, HttpResponse &res, const ServerData &server)
{
    handleUnknownResponse(req, res, server);
}

CgiRequestHandler::CgiRequestHandler(const HttpConfig::Location &locationConfig, const ServerData &server)
    : RequestController(locationConfig, server) {}

CgiRequestHandler::~CgiRequestHandler() {}

void CgiRequestHandler::handle(const HttpRequest &req, HttpResponse &res, const ServerData &server)
{
    handleCgiResponse(req, res, server);
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
    Logger &logger = Logger::getInstance("server.log");

    std::string resourcePath = _serverRoot;
    logger.log("before resolved resource path: " + _serverRoot);
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

void RequestController::serveResource(const std::string &resourcePath, HttpResponse &res, const ServerData &server)
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
        res.generate500InternalServerError("Internal error 500: " + std::string(e.what()), server._root);
    }

    res.ensureContentLength();
    res.logHttpResponse(logger);
}

void RequestController::handleGetResponse(const HttpRequest &req, HttpResponse &res, const ServerData &server)
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
        handleInternalRequest(req, res, server);
        return;
    }

    std::string resourcePath = resolveResourcePath(uri);
    logger.log("Resolved resource path: " + resourcePath);

    if (!hasReadPermissions(resourcePath))
    {
        res.generate403Forbidden("403 Forbidden: Access to the resource is forbidden", server._root);
        logger.log("Error: Access to the resource is forbidden for resourcePath: " + resourcePath);
        return;
    }

    serveResource(resourcePath, res, server);
}

void RequestController::handlePostResponse(const HttpRequest &req, HttpResponse &res, const ServerData &server) {
    Logger &logger = Logger::getInstance("server.log");
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();
    logger.log("Received POST request for URI: " + uri);


    if (!isPostAllowed(req, res, server)) {
        return;
    }

    // Sélectionner la fonction de traitement en fonction du type de contenu
    std::string contentType = req.getHeader("Content-Type");
    if (contentType.find("multipart/form-data") != std::string::npos) {
        handleMultipartFormData(req, res, server);
    } else if (contentType == "text/plain") {
        handleTextData(req, res);
    } else {
        res.generate415UnsupportedMediaType("Unsupported Media Type for post: " + contentType);
        logger.logError("415 Unsupported Media Type for URI: " + uri);
    }

    //Finalisation de la réponse
    res.setHTTPVersion(version);
    res.ensureContentLength();
    setCorsHeaders(res);
}

bool RequestController::isPostAllowed(const HttpRequest &req, HttpResponse &res, const ServerData &server) {
    if (!isMethodAllowed("POST")) {
        res.generate405MethodNotAllowed("POST method not allowed for this location.");
        return false;
    }
    
    if (req.getBody().empty()) {
        res.generate400BadRequest("Bad Request: Empty body or malformed request.");
        return false;
    }

    if (!_locationConfig.allowUploads) {
        res.generate403Forbidden("Forbidden: Uploads are not allowed at this location.", server._root);
        return false;
    }

    std::string uploadsDir = _serverRoot + "/uploads/";
    struct stat dirStat;
    if (stat(uploadsDir.c_str(), &dirStat) != 0 || !S_ISDIR(dirStat.st_mode) || access(uploadsDir.c_str(), W_OK) != 0) {
        res.generate500InternalServerError("Uploads directory does not exist or is not writable.", server._root);
        return false;
    }

    return true;
}

// Gestion des requêtes multipart/form-data
void RequestController::handleMultipartFormData(const HttpRequest &req, HttpResponse &res, const ServerData &server) {
    Logger &logger = Logger::getInstance("server.log");

    try {
    std::string boundary = req.getBoundary();
    if (boundary.empty()) {
        throw std::runtime_error("No boundary found in Content-Type header");
    }

    HttpRequest::FormData formData = req.parseMultipartFormData();
    if (formData.fields.find("file") == formData.fields.end()) {
        throw std::runtime_error("No file found in form data");
    }

    std::string fileContent = formData.fields["file"];
    std::string fileName = formData.fileName;
    /* if (fileName.empty()) {
    throw std::runtime_error("Filename is missing in the form data");
    } */
    std::string filePath = _serverRoot + "/uploads/";
    logger.log("File name " + fileName);

    if (fileName.find("..") != std::string::npos) {
        res.generate403Forbidden("Forbidden: Invalid filename", server._root);
        return;
    }

    filePath += fileName;

    // Vérifiez si le fichier existe
    std::ifstream existingFile(filePath.c_str());
    if (existingFile.good() && !fileName.empty()) {
        existingFile.close();
        res.generate409Conflict("Conflict: The file already exists.");
        logger.log("409 Conflict: File already exists: " + filePath);
        return;
    }
    existingFile.close();

    // Écrire le fichier
    std::ofstream outFile(filePath.c_str(), std::ios::binary);
    if (!outFile.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filePath);
    }
    outFile.write(fileContent.c_str(), fileContent.size());
    outFile.close();

    res.generate201Created("/files/" + fileName);
    logger.log("File uploaded successfully: " + filePath);
    } catch (const std::exception &e) {
        res.generate500InternalServerError("Internal Server Error: " + std::string(e.what()), server._root);
        logger.log("Error processing POST request: " + std::string(e.what()));
    }
}


// Gestion des requêtes text/plain
void RequestController::handleTextData(const HttpRequest &req, HttpResponse &res) {
    Logger &logger = Logger::getInstance("server.log");
    std::string body = req.getBody();

    if (body.empty()) {
        res.generate400BadRequest("Bad Request: Empty text body.");
        logger.logError("400 Bad Request: Empty text body.");
        return;
    }

    res.generate200OK("text/html", "Text data processed successfully.");
    logger.log("Text data processed for URI: " + req.getURI());
}



void RequestController::handleDeleteResponse(const HttpRequest &req, HttpResponse &res, const ServerData &server)
{
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();
    Logger &logger = Logger::getInstance("server.log");
    logger.log("Received DELETE request for URI: " + uri);

    if (!hasPermissionToDelete(uri))
    {
        res.generate403Forbidden("403 Forbidden: You do not have permission to delete this resource.", server._root);
        return;
    }

    // Expected format: /files/filename
    const std::string prefix = "/files/";

    if (uri.compare(0, prefix.length(), prefix) != 0)
    {
        res.generate404NotFound("Invalid URI for DELETE operation: " + uri, server._root);
        return;
    }

    std::string filename = uri.substr(prefix.length());

    // Security check to prevent directory traversal attacks
    if (filename.find("..") != std::string::npos)
    {
        res.generate403Forbidden("Forbidden: Invalid filename", server._root);
        return;
    }

    // Construct the full path to the file in the files directory
    std::string resourcePath = _serverRoot + "/index/files/" + filename;

    // Attempt to delete the file
    if (remove(resourcePath.c_str()) != 0)
    {
        res.generate404NotFound("404 Not Found: Resource not found: " + uri, server._root);
        return;
    }
    logger.log("File deleted successfully: " + resourcePath);
    res.generate204NoContent("204 No Content: Delete success");
    res.setHTTPVersion(version);
}

void RequestController::handleUnknownResponse(const HttpRequest &req, HttpResponse &res, const ServerData &server)
{
    (void)server;
    std::string version = req.getHTTPVersion();
    std::string method = req.getMethod();

    if (method.empty())
    {
        res.generate400BadRequest("400 Bad Request: Method cannot be empty.");
        return;
    }
    res.generate405MethodNotAllowed("405: Method not allowed: " + method);
    // res.generate501NotImplemented("501 Not Implemented: The server does not recognize the HTTP method used: " + method);
    res.setHTTPVersion(version);
}

void RequestController::setCorsHeaders(HttpResponse &res)
{
    res.setHeader("Access-Control-Allow-Origin", "*");                           // Permet toutes les origines
    res.setHeader("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS"); // Méthodes autorisées
    res.setHeader("Access-Control-Allow-Headers", "Content-Type");               // En-têtes autorisés
}

void RequestController::handleInternalRequest(const HttpRequest &req, HttpResponse &res, const ServerData &server)
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
        res.generate404NotFound("Invalid internal URI: " + uri, server._root);
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

void RequestController::handleOptionsResponse(const HttpRequest &req, HttpResponse &res, const ServerData &server)
{
    (void)server;
    (void)req;
    res.setStatusCode(204); // No Content
    res.setReasonMessage("No Content");
    res.setHeader("Access-Control-Allow-Origin", "*");
    res.setHeader("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
    res.setHeader("Access-Control-Allow-Headers", "Content-Type");
    res.setHeader("Access-Control-Max-Age", "86400");
    res.setBody("");
}

void RequestController::handleCgiResponse(const HttpRequest &req, HttpResponse &res, const ServerData &server)
{
    Logger &logger = Logger::getInstance("server.log");
    std::string cgiScriptPath = resolveCgiPath();
    logger.log("Received CGI request for URI: " + req.getURI());

    if (!isCgiExecutable(cgiScriptPath, res))
        return;

    std::vector<std::string> envVariables = setupEnvironmentVariables(req, cgiScriptPath);

    int stdin_pipe[2], stdout_pipe[2];
    if (!createPipes(stdin_pipe, stdout_pipe, res, logger, server))
        return;

    pid_t pid = fork();
    if (pid < 0)
    {
        res.generate500InternalServerError("500 Internal Server Error: Failed to fork process", server._root);
        logger.log("Error: Failed to fork process for CGI execution");
        return;
    }
    else if (pid == 0)
    {
        executeCgiScript(cgiScriptPath, envVariables, stdin_pipe, stdout_pipe);
    }
    else
    {
        processCgiOutput(pid, stdin_pipe, stdout_pipe, req, res, server);
    }

    res.ensureContentLength();
    setCorsHeaders(res);
}

// Résolution du chemin CGI
std::string RequestController::resolveCgiPath()
{
    return _serverRoot + "/" + getHandler(); // Ajuste le chemin si nécessaire
}

// Vérifie si le script CGI est exécutable
bool RequestController::isCgiExecutable(const std::string &cgiScriptPath, HttpResponse &res)
{
    struct stat scriptStat;
    Logger &logger = Logger::getInstance("server.log");
    if (stat(cgiScriptPath.c_str(), &scriptStat) != 0 || !S_ISREG(scriptStat.st_mode) || !(scriptStat.st_mode & S_IXUSR))
    {
        res.generate403Forbidden("403 Forbidden: CGI script is not accessible or does not exist", _server._root);
        logger.log("Error: CGI script not found or not executable: " + cgiScriptPath);
        return false;
    }
    return true;
}

// Configure les variables d'environnement pour le script CGI
std::vector<std::string> RequestController::setupEnvironmentVariables(const HttpRequest &req, const std::string &cgiScriptPath)
{
    std::vector<std::string> envVariables;
    envVariables.push_back("REQUEST_METHOD=" + req.getMethod());
    envVariables.push_back("QUERY_STRING=" + req.getQueryParameters());
    envVariables.push_back("CONTENT_TYPE=" + req.getContentType());
    envVariables.push_back("CONTENT_LENGTH=" + to_string(req.getBody().length()));

    size_t lastSlash = cgiScriptPath.find_last_of("/\\");
    std::string filename = cgiScriptPath.substr(lastSlash + 1);
    envVariables.push_back("SCRIPT_NAME=" + filename);
    envVariables.push_back("SCRIPT_PATH=" + cgiScriptPath);
    return envVariables;
}

// Crée les pipes pour stdin et stdout
bool RequestController::createPipes(int stdin_pipe[2], int stdout_pipe[2], HttpResponse &res, Logger &logger, const ServerData &server)
{
    if (pipe(stdin_pipe) == -1 || pipe(stdout_pipe) == -1)
    {
        res.generate500InternalServerError("500 Internal Server Error: Failed to create pipes", server._root);
        logger.log("Error: Failed to create pipes for CGI execution");
        return false;
    }
    return true;
}

// Gère l'exécution du script CGI
void RequestController::executeCgiScript(const std::string &cgiScriptPath, const std::vector<std::string> &envVariables, int stdin_pipe[2], int stdout_pipe[2])
{
    Logger &logger = Logger::getInstance("server.log");
    std::string cgiHandler = _locationConfig.cgiHandler;
    struct stat scriptStat;
    close(stdin_pipe[1]);
    dup2(stdin_pipe[0], STDIN_FILENO);
    close(stdin_pipe[0]);

    close(stdout_pipe[0]);
    dup2(stdout_pipe[1], STDOUT_FILENO);
    dup2(stdout_pipe[1], STDERR_FILENO); // Redirect stderr to stdout
    close(stdout_pipe[1]);

    // Set environment variables
    char *envp[envVariables.size() + 1];
    for (size_t i = 0; i < envVariables.size(); ++i)
    {
        envp[i] = const_cast<char *>(envVariables[i].c_str());
    }
    envp[envVariables.size()] = nullptr;

    logger.log("CGI cgiScriptPath: " + cgiScriptPath);
    logger.log("CGI cgiHandler: " + cgiHandler);

    char *argv[] = {const_cast<char *>(cgiHandler.c_str()), const_cast<char *>(cgiScriptPath.c_str()), nullptr};

    if (stat(cgiScriptPath.c_str(), &scriptStat) == 0 && (scriptStat.st_mode & S_IXUSR))
    {
        // Script is executable, execute it directly
        execve(cgiScriptPath.c_str(), argv, envp);
    }
    else if (!cgiHandler.empty())
    {
        // Use the specified interpreter
        execve(cgiHandler.c_str(), argv, envp);
    }
    else
    {
        // No interpreter specified and script is not executable
        logger.logError("No CGI handler specified and script is not executable.");
        exit(1);
    }

    perror("execve failed");
    exit(1);
}

// Gère la lecture de la sortie CGI et la réponse HTTP
void RequestController::processCgiOutput(pid_t pid, int stdin_pipe[2], int stdout_pipe[2], const HttpRequest &req, HttpResponse &res, const ServerData &server)
{
    Logger &logger = Logger::getInstance("server.log");
    close(stdin_pipe[0]);
    close(stdout_pipe[1]);

    write(stdin_pipe[1], req.getBody().c_str(), req.getBody().size());
    close(stdin_pipe[1]);
    // logger.log("request BODY : " + req.getBody());

    std::string output;
    char buffer[4096];
    ssize_t bytesRead;
    while ((bytesRead = read(stdout_pipe[0], buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytesRead] = '\0';
        output += buffer;
    }
    close(stdout_pipe[0]);

    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
    {
        handleCgiResponseOutput(output, res);
    }
    else
    {
        res.generate500InternalServerError("500 Internal Server Error: CGI script execution failed", server._root);
        logger.log("Error: CGI script execution failed with status: " + to_string(WEXITSTATUS(status)));
    }
    logger.log("CGI output: " + output);
}

// Analyse la sortie du script CGI et met à jour la réponse HTTP
void RequestController::handleCgiResponseOutput(const std::string &output, HttpResponse &res)
{
    // Logger &logger = Logger::getInstance("server.log");
    size_t headerEndPos = output.find("\r\n\r\n");
    if (headerEndPos == std::string::npos)
    {
        headerEndPos = output.find("\n\n");
    }

    if (headerEndPos != std::string::npos)
    {
        std::string headers = output.substr(0, headerEndPos);
        std::string body = output.substr(headerEndPos + 4);

        std::istringstream headerStream(headers);
        std::string headerLine;
        while (std::getline(headerStream, headerLine))
        {
            if (!headerLine.empty() && headerLine.back() == '\r')
                headerLine.pop_back();

            size_t colonPos = headerLine.find(':');
            if (colonPos != std::string::npos)
            {
                std::string headerName = headerLine.substr(0, colonPos);
                std::string headerValue = headerLine.substr(colonPos + 1);
                res.setHeader(headerName, headerValue);
            }
        }

        res.setBody(body);
        // logger.log("BODY : " + body);
        res.setStatusCode(200);
        res.setReasonMessage("OK");
    }
    else
    {
        res.generate200OK("text/html", output);
    }
    // logger.log("CGI script executed successfully: " + output);
}

std::string RequestController::getHandler()
{
    return (_locationConfig.handler);
}

ServerData &RequestController::getServerInfo()
{
    return (_server);
}
