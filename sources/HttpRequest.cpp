#include "HttpRequest.hpp"
#include <sstream> // pour std::ostringstream

std::string HttpRequest::trim(const std::string &str)
{
    size_t first = str.find_first_not_of(' ');
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

HttpRequest::HttpRequest()
    : _method(""), _uri(""), _version("HTTP/1.1"), _headers(), _body(""), _queryParameters(""), _allowedMethods(initMethods())
{
    // Do nothing else; no parsing
}

HttpRequest::HttpRequest(const std::string &rawData)
    : _method(""), _uri(""), _version("HTTP/1.1"), _headers(), _body(""), _queryParameters(""), _allowedMethods(initMethods()), _contentType("")
{
    size_t headerEndPos = rawData.find("\r\n\r\n");
    if (headerEndPos == std::string::npos)
    {
        throw std::runtime_error("Invalid HTTP request: Missing headers");
    }
    std::string headerPart = rawData.substr(0, headerEndPos);
    _body = rawData.substr(headerEndPos + 4);

    std::istringstream headerStream(headerPart);
    std::string requestLine;
    if (!std::getline(headerStream, requestLine))
    {
        throw std::runtime_error("Invalid request line");
    }
    if (!requestLine.empty() && requestLine[requestLine.length() - 1] == '\r')
    {
        requestLine = requestLine.substr(0, requestLine.length() - 1);
    }
    std::istringstream requestLineStream(requestLine);
    requestLineStream >> _method >> _uri >> _version;

    if (_method.empty() || _uri.empty() || _version.empty())
    {
        throw std::runtime_error("Invalid request line");
    }

    size_t queryPos = _uri.find('?');
    if (queryPos != std::string::npos)
    {
        _queryParameters = _uri.substr(queryPos + 1);
        _uri = _uri.substr(0, queryPos);
    }

    std::string headerLine;
    while (std::getline(headerStream, headerLine))
    {
        if (!headerLine.empty() && headerLine[headerLine.length() - 1] == '\r')
            headerLine = headerLine.substr(0, headerLine.length() - 1);

        if (headerLine.empty()) break;

        size_t colonPos = headerLine.find(':');
        if (colonPos != std::string::npos)
        {
            std::string headerName = trim(headerLine.substr(0, colonPos));
            std::string headerValue = trim(headerLine.substr(colonPos + 1));
            _headers[headerName] = headerValue;
        }
    }

    _contentType = getHeader("Content-Type");

    std::map<std::string, std::string>::iterator contentLengthIt = _headers.find("Content-Length");
    if (contentLengthIt != _headers.end())
    {
        std::istringstream iss(contentLengthIt->second);
        size_t contentLength;
        iss >> contentLength;
        if (_body.size() < contentLength)
        {
            throw std::runtime_error("Incomplete request body");
        }
        else if (_body.size() > contentLength)
        {
            _body = _body.substr(0, contentLength);
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
std::string HttpRequest::getContentType() const { return getHeader("Content-Type"); }
bool HttpRequest::isChunked() const { return false; }
std::string HttpRequest::getFileName() const { return _fileName; }
bool HttpRequest::isCgi() const { return isCgiRequest; }

bool HttpRequest::isMethodAllowed(const std::string &method) const
{
    return _allowedMethods.find(method) != _allowedMethods.end();
}

std::set<std::string> HttpRequest::initMethods()
{
    std::set<std::string> methods;
    methods.insert("GET");
    methods.insert("POST");
    methods.insert("DELETE");
    methods.insert("UNKNOWN");
    methods.insert("CGI");
    return methods;
}

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

void HttpRequest::setMethod(std::string method) { _method = method; }
void HttpRequest::setURI(std::string uri) { _uri = uri; }
void HttpRequest::setVersion(std::string version) { _version = version; }
void HttpRequest::setCgi(bool cgi) { isCgiRequest = cgi; }

std::string HttpRequest::getBoundary() const
{
    std::string contentType = getHeader("Content-Type");
    std::string boundaryPrefix = "boundary=";
    size_t pos = contentType.find(boundaryPrefix);
    if (pos != std::string::npos)
    {
        return "--" + contentType.substr(pos + boundaryPrefix.length());
    }
    return "";
}

HttpRequest::FormData HttpRequest::parseMultipartFormData() const
{
    FormData result;
    std::map<std::string, std::string> formData;
    std::string boundary = getBoundary();
    if (boundary.empty())
    {
        throw std::runtime_error("No boundary found in Content-Type header");
    }

    size_t pos = 0;
    while ((pos = _body.find(boundary, pos)) != std::string::npos)
    {
        size_t endPos = _body.find(boundary, pos + boundary.length());
        if (endPos == std::string::npos)
        {
            endPos = _body.length();
        }
        std::string part = _body.substr(pos + boundary.length(), endPos - pos - boundary.length());
        size_t headerEnd = part.find("\r\n\r\n");
        if (headerEnd != std::string::npos)
        {
            std::string partHeaders = part.substr(0, headerEnd);
            std::string partContent = part.substr(headerEnd + 4);

            std::istringstream partHeaderStream(partHeaders);
            std::string headerLine;
            std::string disposition;
            while (std::getline(partHeaderStream, headerLine))
            {
                if (!headerLine.empty() && headerLine[headerLine.length() - 1] == '\r')
                    headerLine = headerLine.substr(0, headerLine.length() - 1);

                if (headerLine.find("Content-Disposition:") != std::string::npos)
                {
                    disposition = headerLine;
                }
            }

            size_t namePos = disposition.find("name=\"");
            if (namePos != std::string::npos)
            {
                namePos += 6;
                size_t nameEnd = disposition.find("\"", namePos);
                std::string name = disposition.substr(namePos, nameEnd - namePos);

                size_t filenamePos = disposition.find("filename=\"");
                if (filenamePos != std::string::npos)
                {
                    filenamePos += 10;
                    size_t filenameEnd = disposition.find("\"", filenamePos);
                    result.fileName = disposition.substr(filenamePos, filenameEnd - filenamePos);
                }

                result.fields[name] = partContent;
            }
        }
        pos = endPos;
    }
    return result;
}
