#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include <iostream>
#include <map>
#include <string>

// Handle creation and representation of an HTTP Response
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

        void setHeader(const std::string& name, const std::string& value);
        std::string getStatusMessage() const;
        std::string toString() const;
        void ensureContentLength();
};

#endif
