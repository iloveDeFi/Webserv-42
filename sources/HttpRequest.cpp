#include "HttpRequest.hpp"

std::string HttpRequest::trim(const std::string &str)
{
    size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first)
    {
        return "";
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

// Constructeur pour analyser la raw data
HttpRequest::HttpRequest(const std::string &rawData)
    : _method(""), _uri(""), _version("HTTP/1.1"), _headers(), _body(""), _queryParameters(""), _allowedMethods(initMethods())
{
    // Convertit les données brutes en un flux de texte pour analyse
    std::istringstream requestStream(rawData);
    std::string requestLine;

    // Analyse la première ligne de la requête (ligne de requête)
    if (std::getline(requestStream, requestLine))
    {
        std::istringstream lineStream(requestLine);
        lineStream >> _method >> _uri >> _version;

        // Validation de la ligne de requête
        if (_method.empty() || _uri.empty() || _version.empty())
        {
            throw std::runtime_error("Invalid request line");
        }

        // Si l'URI contient des paramètres de requête après un '?', les extraire
        size_t queryPos = _uri.find('?');
        if (queryPos != std::string::npos)
        {
            _queryParameters = _uri.substr(queryPos + 1); // Stocke les paramètres après '?'
            _uri = _uri.substr(0, queryPos); // Récupère juste l'URI sans les paramètres
        }
    }

    // Analyse des headers
    std::string headerLine;
    while (std::getline(requestStream, headerLine) && !headerLine.empty())
    {
        size_t colonPos = headerLine.find(':');
        if (colonPos != std::string::npos)
        {
            std::string headerName = trim(headerLine.substr(0, colonPos));
            std::string headerValue = trim(headerLine.substr(colonPos + 1));
            _headers[headerName] = headerValue; // Stocke les headers dans la map
        }
    }

    // Vérification de la longueur du corps (Content-Length) pour lire le corps de la requête
    std::map<std::string, std::string>::iterator contentLengthIt = _headers.find("Content-Length");
    if (contentLengthIt != _headers.end())
    {
        int contentLength = atoi(contentLengthIt->second.c_str());
        if (contentLength > 0)
        {
            char *bodyData = new char[contentLength + 1];
            requestStream.read(bodyData, contentLength);
            bodyData[contentLength] = '\0';
            _body = std::string(bodyData); // Stocke le corps de la requête
            delete[] bodyData;
        }
    }
}

HttpRequest::~HttpRequest() {}

HttpRequest::HttpRequest(const HttpRequest &src)
    : _method(src._method), _uri(src._uri), _version(src._version), _headers(src._headers), _body(src._body), _queryParameters(src._queryParameters), _allowedMethods(src._allowedMethods) {}

HttpRequest &HttpRequest::operator=(const HttpRequest &src)
{
    if (this != &src)
    {
        _method = src._method;
        _uri = src._uri;
        _version = src._version;
        _headers = src._headers;
        _body = src._body;
        _queryParameters = src._queryParameters;
        _allowedMethods = src._allowedMethods;
    }
    return *this;
}

// Méthodes pour obtenir les valeurs
std::string HttpRequest::getMethod() const { return _method; }
std::string HttpRequest::getURI() const { return _uri; }
std::string HttpRequest::getHTTPVersion() const { return _version; }
std::map<std::string, std::string> HttpRequest::getHeaders() const { return _headers; }
std::string HttpRequest::getHeader(const std::string &name) const
{
    std::map<std::string, std::string>::const_iterator it = _headers.find(name);
    if (it != _headers.end())
    {
        return it->second;
    }
    return "";
}
std::string HttpRequest::getBody() const { return _body; }
std::string HttpRequest::getQueryParameters() const { return _queryParameters; }
bool HttpRequest::isChunked() const { return false; }

// Méthode pour valider si une méthode est autorisée
bool HttpRequest::isMethodAllowed(const std::string &method) const
{
    return _allowedMethods.find(method) != _allowedMethods.end();
}

// Autres méthodes comme l'initialisation des méthodes autorisées
std::set<std::string> HttpRequest::initMethods()
{
    std::set<std::string> methods;
    methods.insert("GET");
    methods.insert("POST");
    methods.insert("DELETE");
    return methods;
}

// Log de la requête HTTP
void HttpRequest::logHttpRequest(Logger &logger)
{
    std::ostringstream logMessage;
    logMessage << "HTTP Request:\n";
    logMessage << "Method: " << _method << "\n";
    logMessage << "URI: " << _uri << "\n";
    logMessage << "HTTP Version: " << _version << "\n";
    logMessage << "Headers:\n";
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
    {
        logMessage << it->first << ": " << it->second << "\n";
    }
    logMessage << "Body: " << _body << "\n";
    logMessage << "Query Parameters: " << _queryParameters << "\n";
    logger.log(logMessage.str());
}

// Définir la méthode, URI, et version si besoin
void HttpRequest::setMethod(std::string method)
{
    _method = method;
}
void HttpRequest::setURI(std::string uri)
{
    _uri = uri;
}
void HttpRequest::setVersion(std::string version)
{
    _version = version;
}
