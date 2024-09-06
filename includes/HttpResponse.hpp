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

        // Setter
        void setHeader(const std::string& name, const std::string& value);
        // Getter
        std::string getStatusMessage() const;
        // Other Methods
        std::string toString() const;
        void ensureContentLength();
        // Response
        std::string generate404Error(const std::string& uri);
        std::string generateRedirection(const std::string& newUri);
};

#endif
