#ifndef HTTPCONTROLLER_HPP
# define HTTPCONTROLLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <string>
#include <map>
class RequestController {
	private:
		std::map<std::string, std::string>& _resourceDatabase;
    public:
        //TO DO : need coplien's form if memory management needed ex. dynamic
        RequestController();
		//RequestController(std::map<std::string, std::string> RsrDatabase);
        //~RequestController();
        RequestController(const RequestController& src);
        RequestController& operator=(const RequestController& src);
        virtual void handle(const HttpRequest& req, HttpResponse& res) = 0;
        virtual ~RequestController();
		std::map<std::string, std::string>& getResourceDatabase(){
			return _resourceDatabase;
		}
};
class GetRequestHandler : public RequestController {
    public:
		GetRequestHandler();
		~GetRequestHandler();
        void handle(const HttpRequest& req, HttpResponse& res);
};
class PostRequestHandler : public RequestController {
    public:
		PostRequestHandler();
		~PostRequestHandler();
        void handle(const HttpRequest& req, HttpResponse& res);
};
class DeleteRequestHandler : public RequestController {
	public:
		DeleteRequestHandler();
		~DeleteRequestHandler();
        void handle(const HttpRequest& req, HttpResponse& res);
};

#endif
