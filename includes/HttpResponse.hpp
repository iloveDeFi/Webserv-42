#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include "HttpRequest.hpp"
#include <iostream>
#include <map>
#include <string>
class HttpResponse {
    private:
        std::string _httpVersion;
        int _statusCode;
        std::string _statusMessage;
        std::string _body;
        std::string _version;
        std::map<std::string, std::string> _headers;
        std::string _body;

    public:
        HttpResponse();
        ~HttpResponse();
        HttpResponse(const HttpResponse& src);
        HttpResponse& operator=(const HttpResponse& src);
        
        // 1) RESPONSE LINE
        std::set<std::string> initMethods() const;
        // void setMethod(const std::string& method);
        // bool isMethodAllowed(const std::string& method) const;
        // void setURI(const std::string& uri);
        void setHTTPVersion(const std::string& _httpVersion);
        void setStatusCode(int _statusCode);
        std::string setStatusMessage();
        // 2) HEADERS
        void setHeader(const std::string& name, const std::string& value);
        void setHeaders(const std::map<std::string, std::string>& headers);
        void addHeader(std::string, std::string);
        // 3) BODY
        void setBody(const std::string& body);
        void setIsChunked(bool is_chunked);
        void ensureContentLength();
        // 4) TEMPLATE
        std::string generate404Error(const std::string& uri);
        std::string generateRedirection(const std::string& newUri);
        // 5) OTHERS
        std::string responseToString() const;
};      

#endif
