#ifndef HTTPCONTROLLER_HPP
# define HTTPCONTROLLER_HPP

#include "HttpRequest.hpp"
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

#endif