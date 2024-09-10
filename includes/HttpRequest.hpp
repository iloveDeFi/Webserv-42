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
// 2) HTTP Request HeaderS : NAME OR VALUE
// 3) (empty line)
// 4) HTTP Request Body

class HttpRequest {
    private:
        std::string _method;
        std::string _uri;
        std::string _version;
        std::map<std::string, std::string> _headers;
        std::set<std::string> _allowedMethods;
        std::string _body;
        std::string _queryParameters;
        std::string _cookies;
        bool _isChunked;
        static const std::set<std::string> initMethods();
        
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
        // 3) BODY
        std::string getBody() const;
        std::string getQueryParameters() const;
        std::string getCookies() const;
        bool isChunked() const;

        // Other methods
        bool isMethodAllowed(const std::string& method) const;
        void requestController(HttpResponse& response, std::map<std::string, std::string>& resourceDatabase);
};

std::ostream&	operator<<(std::ostream& os, const HttpRequest& re);

#endif
