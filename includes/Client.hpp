
#ifndef CLIENT_HPP
# define CLIENT_HPP
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"
# include "Location.hpp"
# include <fstream>
# include <unistd.h>
# include <sys/socket.h>


class Client
{
	private:
		int _socket;
		std::string _ipAddress;
		HttpRequest _request;
		HttpResponse _response;
		
	public:
		Client(int socket, int ip);
		/* Client(const Client &other) = delete;
		Client& operator=(const Client &other) = delete; */
		~Client();

		void readRequest(const std::string &rawData);
		void processRequest(std::map<std::string, Location>& location, int size);
		void sendResponse();

		void setHttpRequest(const HttpRequest& request);
		void setHttpResponse(const HttpResponse& response);
		
		HttpRequest& getHttpRequest();
		HttpResponse& getHttpResponse();
		int	getClientSocket();

};


#endif