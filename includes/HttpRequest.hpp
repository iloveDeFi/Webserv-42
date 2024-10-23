#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

class HttpRequest;
class HttpResponse;

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include "HttpResponse.hpp"
#include "HttpController.hpp"
#include "Logger.hpp"
#include <sstream>


class HttpRequest
{

public:
    struct FormData
    {
        std::map<std::string, std::string> fields;
        std::string fileName;
    };
private:
    std::string _method;
    std::string _uri;
    std::string _version;
    std::map<std::string, std::string> _headers;
    std::string _body;
    std::string _queryParameters;
    std::set<std::string> initMethods();
    std::set<std::string> _allowedMethods;
    std::string _fileName;
    bool isCgiRequest;
    std::string _contentType;

public:
    HttpRequest();
    HttpRequest(const std::string &rawData);
    ~HttpRequest();
    HttpRequest(const HttpRequest &src);
    HttpRequest &operator=(const HttpRequest &src);

    std::string getMethod() const;
    std::string getURI() const;
    std::string getBody() const;
    std::string getHTTPVersion() const;
    std::string getContentType() const;
    std::string getQueryParameters() const;

    std::map<std::string, std::string> getHeaders() const;
    std::string getHeader(const std::string &name) const;
    std::string getFileName() const;

    bool isChunked() const;

    bool isMethodAllowed(const std::string &method) const;
    bool isSupportedContentType(const std::string &contentType) const;
    void requestController(HttpResponse &response);
    std::string trim(const std::string &str);

    void logHttpRequest(Logger &logger);

    void setMethod(std::string method);
    void setURI(std::string uri);
    void setVersion(std::string version);
    std::string getBoundary() const;
    FormData parseMultipartFormData() const;
    
    bool isCgi() const;
    void setCgi(bool cgi);
};

std::ostream &operator<<(std::ostream &os, const HttpRequest &re);

#endif
