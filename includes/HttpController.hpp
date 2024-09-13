#ifndef HTTPCONTROLLER_HPP
# define HTTPCONTROLLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <string>
#include <map>
class RequestController {
    public:
        // TO DO : need coplien's form if memory management needed ex. dynamic
        // RequestController();
        // ~RequestController();
        // RequestController(const RequestController& src);
        // RequestController& operator=(const RequestController& src);
        virtual void handle(const HttpRequest& req, HttpResponse& res) = 0;
        virtual ~RequestController() {}
};
class GetRequestHandler : public RequestController {
    public:
        void handle(const HttpRequest& req, HttpResponse& res);
};
class PostRequestHandler : public RequestController {
    public:
        void handle(const HttpRequest& req, HttpResponse& res);
};
class DeleteRequestHandler : public RequestController {
    public:
        void handle(const HttpRequest& req, HttpResponse& res);
};

#endif
