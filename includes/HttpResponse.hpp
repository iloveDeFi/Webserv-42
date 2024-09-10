#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include <iostream>
#include <map>
#include <string>
#include "HttpRequest.hpp"

// My response format is:
// HTTP Response Line
// HTTP Response Headers
// (empty line)
// HTTP Response Body
class HttpResponse {
    private:
        int _statusCode;
        std::string _version;
        std::map<std::string, std::string> _headers;
        std::string _body;

    public:
        HttpResponse();
        ~HttpResponse();
        HttpResponse(const HttpResponse& src);
        HttpResponse& operator=(const HttpResponse& src);

         // Setters
        // 1) response line
        std::string setHTTPVersion(const std::string& version);
        std::string setMethod(const std::string& method);
        std::string setURI(const std::string& uri);
        
        // 2) headers
        std::map<std::string, std::string> setHeaders();
        std::string setHeader(const std::string& name, const std::string& value);
        std::string setHeadersAsString();
        // 3) body
        std::string setBody();
        std::string setQueryParameters();
        std::string setCookies();
        bool isChunked();

        // Utils Methods
        std::string toString();
        void ensureContentLength();

        // Response
        std::string generate404Error(const std::string& uri);
        std::string generateRedirection(const std::string& newUri);
};

std::ostream&	operator<<(std::ostream& os, const HttpResponse& re);

#endif
