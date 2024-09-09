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
        std::map<std::string, std::string> _headers;
        std::set<std::string> _allowedMethods;
        std::string _body;
        bool _is_chunked;
        
    public:
        HttpRequest();
        ~HttpRequest();
        HttpRequest(const HttpRequest& src);
        HttpRequest& operator=(const HttpRequest& src);

         // Getters
        std::string getMethod() const;
        std::string getURI() const;
        std::string getHTTPVersion() const;
        std::map<std::string, std::string> getHeaders() const;
        std::string getHeader(const std::string& name) const; // check here
        std::string getBody() const;
        bool isChunked() const;

        // Setter (si nécessaire pour simuler des données)
        void setMethod(const std::string& method);
        void setURI(const std::string& uri);
        void setHTTPVersion(const std::string& version);
        void setHeaders(const std::map<std::string, std::string>& headers);
        void setBody(const std::string& body);
        void setIsChunked(bool is_chunked);

        // Other Methods
        std::set<std::string> initMethods() const;
        bool isMethodAllowed(const std::string& method) const;
        // TO do : Controller to handle request
        void requestController(const HttpRequest& request, HttpResponse& response);
};

std::ostream&	operator<<(std::ostream& os, const HttpRequest& re);

#endif
