
#ifndef CLIENT_HPP
# define CLIENT_HPP
# include "server.hpp"
# include "Socket.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"

class Client
{
	private:
		int _socket;
		HttpRequest _request;
		HttpResponse _response;
		
	public:
		Client(int socket);
		Client(const Client &other) = delete;
		Client& operator=(const Client &other) = delete;
		~Client();

		void readRequest(const std::string &rawData);
		void processRequest();
		void sendResponse();

		void setHttpRequest(const HttpRequest& request);
		void setHttpResponse(const HttpResponse& response);
		
		HttpRequest& getHttpRequest();
		HttpResponse& getHttpResponse();
		int	getClientSocket();

};


#endif