#ifndef CLIENT_HPP
# define CLIENT_HPP
#include "HttpController.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
# include "Location.hpp"
# include <fstream>
# include <unistd.h>
# include <sys/socket.h>
# include <netinet/in.h>
#include <arpa/inet.h>//to remove if no ip address


class Client
{
	private:
		int _socket;
		struct sockaddr_in _address;
		HttpRequest _request;
		HttpResponse _response;
		
	public:
		Client(int socket, const struct sockaddr_in& address);
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
		std::string getIPaddress();
		bool isConnected() const;
};


#endif