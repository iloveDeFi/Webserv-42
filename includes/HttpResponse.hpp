#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include <iostream>
#include <map>
#include <string>

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
        bool _isChunked;

    public:
        HttpResponse();
        ~HttpResponse();
        HttpResponse(const HttpResponse& src);
        HttpResponse& operator=(const HttpResponse& src);

         // Setters
        // 1) response line
        void setHTTPVersion(const std::string& version);
        void setStatusCode(int status);
        // 2) headers
        void setHeader(const std::string& name, const std::string& value);
        void setHeaders(const std::map<std::string, std::string>& headers);
        std::string addHeader(std::string allow, std::string method) const;
        // 3) body
        void setBody(const std::string& body);
        void setIsChunked(bool isChunked);

        // Utils Methods
        std::string toString() const;
        void ensureContentLength();

        // Response templates
        std::string generate404Error(const std::string& uri);
        std::string generateRedirection(const std::string& newUri);
};

std::ostream& operator<<(std::ostream& os, const HttpResponse& re);

#endif
