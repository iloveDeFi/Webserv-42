#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

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

class HttpRequest {
    private:
        // 1) REQUEST LINE
        std::string _method;
        std::string _uri;
        std::string _version;

        // 2) HEADERS
        std::string _name;
        std::string _value;
        std::map<std::string, std::string> _headers;
        std::string _contentType;
        int _contentLength;
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
        std::string _cookies;
        bool _isChunked;

        // OTHER
        static const std::set<std::string> initMethods();
        std::set<std::string> _allowedMethods;
        // TO DO : add following in server class i guess
        // std::map<std::string, std::string>& resourceDatabase;
        
    public:
        HttpRequest();
        ~HttpRequest();
        HttpRequest(const HttpRequest& src);
        HttpRequest& operator=(const HttpRequest& src);

        // Getters
        // 1) REQUEST LINE
        std::string getMethod() const;
        std::string getURI() const;
        std::string getHTTPVersion() const;

        // 2) HEADERS
        std::map<std::string, std::string> getHeaders() const;
        std::string getHeader(const std::string& name) const;
        std::string getContentType(std::string contentType) const;
        int getContentLength(int contentLength) const;

        // 3) BODY
        std::string getBody() const;
        std::string getQueryParameters() const;
        std::string getCookies() const;
        bool isChunked() const;

        // OTHER
        bool isMethodAllowed(const std::string& method) const;
};

// PRINT DATA
std::ostream&	operator<<(std::ostream& os, const HttpRequest& re);

#endif
