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
#include <sstream>
class HttpRequest
{
private:
    std::string _method;
    std::string _uri;
    std::string _version;
    std::map<std::string, std::string> _headers;
    std::string _body;
    std::string _queryParameters;
    std::set<std::string> initMethods();
    std::set<std::string> _allowedMethods;

public:
    HttpRequest(const std::string &rawData);
    ~HttpRequest();
    HttpRequest(const HttpRequest &src);
    HttpRequest &operator=(const HttpRequest &src);

    std::string getMethod() const;
    std::string getURI() const;
    std::string getHTTPVersion() const;

    std::map<std::string, std::string> getHeaders() const;
    std::string getHeader(const std::string &name) const;

    std::string getBody() const;
    std::string getQueryParameters() const;
    bool isChunked() const;

    bool isMethodAllowed(const std::string &method) const;
    bool isSupportedContentType(const std::string &contentType) const;
    void requestController(HttpResponse &response);
    std::string trim(const std::string &str);
};

std::ostream &operator<<(std::ostream &os, const HttpRequest &re);

#endif
