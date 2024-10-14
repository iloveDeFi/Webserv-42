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

/// My request format is:
// 1) HTTP Request Line : METHOD_VERB/CHEMIN_RESSOURCE/PROTOCOL_HTPP
// 2) HTTP Request Headers : NAME OR VALUE
// X) (empty line)
// 3) HTTP Request Body

class HttpRequest
{
private:
    // 1) REQUEST LINE
    std::string _method;
    std::string _uri;
    std::string _version;

    // 2) HEADERS
    std::map<std::string, std::string> _headers;

    // TO DO MAYBE LATER ALIGATOR :
    // accept
    // accept-language
    // accept-encoding
    // connection
    // cookie
    // host
    // user-agent

    // 3) BODY
    std::string _body;
    std::string _queryParameters;
    // TO DO IF BONUS :
    // std::string _cookies;

    // OTHER
    /* static const  */ std::set<std::string> initMethods();
    std::set<std::string> _allowedMethods;
    // TO DO : add following in server class i guess
    // std::map<std::string, std::string>& resourceDatabase;

    // TO DO LATER ALIGATOR : MULTI ?

public:
    HttpRequest(const std::string &rawData);
    ~HttpRequest();
    HttpRequest(const HttpRequest &src);
    HttpRequest &operator=(const HttpRequest &src);

    // Getters
    // 1) REQUEST LINE
    std::string getMethod() const;
    std::string getURI() const;
    std::string getHTTPVersion() const;

    // 2) HEADERS
    std::map<std::string, std::string> getHeaders() const;
    std::string getHeader(const std::string &name) const;

    // 3) BODY
    std::string getBody() const;
    std::string getQueryParameters() const;
    // std::string getCookies() /* const */;
    bool isChunked() const;

    // OTHER
    bool isMethodAllowed(const std::string &method) const;
    bool isSupportedContentType(const std::string &contentType) const;

    void requestController(HttpResponse &response, std::map<std::string, std::string> &resourceDatabase);
};

// PRINT DATA
std::ostream &operator<<(std::ostream &os, const HttpRequest &re);

#endif
