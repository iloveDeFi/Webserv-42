#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include "HttpResponse.hpp"


class RequestController {
    public:
        virtual void handle(const HttpRequest& req, HttpResponse& res) = 0;
        virtual ~RequestController() {}
};

class GetRequestHandler : public RequestController {
    public:
        void handle(const HttpRequest& req, HttpResponse& res);
};

class HeadRequestHandler : public RequestController {
    public:
        void handle(const HttpRequest& req, HttpResponse& res);
};

class PostRequestHandler : public RequestController {
    public:
        void handle(const HttpRequest& req, HttpResponse& res);
};

class PutRequestHandler : public RequestController {
    public:
        void handle(const HttpRequest& req, HttpResponse& res);
};

class DeleteRequestHandler : public RequestController {
    public:
        void handle(const HttpRequest& req, HttpResponse& res);
};

class ConnectRequestHandler : public RequestController {
    public:
        void handle(const HttpRequest& req, HttpResponse& res);
};

class OptionsRequestHandler : public RequestController {
    public:
        void handle(const HttpRequest& req, HttpResponse& res);
};

class PatchRequestHandler : public RequestController {
    public:
        void handle(const HttpRequest& req, HttpResponse& res);
};

class UnsupportedRequestHandler : public RequestController {
    public:
        void handle(const HttpRequest& req, HttpResponse& res);
};


// Gère l'analyse et la représentation d'une requête HTTP.
class HttpRequest {
    private:
        std::string _method;
        std::string _uri;
        std::string _version;
        std::map<std::string, std::string> _headersAsString;
        std::set<std::string> _allowedMethods;
        std::string _body;
        bool _is_chunked;
        
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
        std::string HttpRequest::getHeadersAsString() const;
        // 3) BODY
        std::string getBody() const;
        std::string HttpRequest::getQueryParameters() const;
        std::string HttpRequest::getCookies() const;
        bool isChunked() const;

        // Controller to handle request
        void requestController(const HttpRequest& request, HttpResponse& response);
};

std::ostream&	operator<<(std::ostream& os, const HttpRequest& re);

#endif
