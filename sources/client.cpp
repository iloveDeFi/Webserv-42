

#include "Client.hpp"

Client::Client(int fd, const struct sockaddr_in& address): _socket(fd), _address(address) {}

/* Client::Client(const Client & src)
: _request(src._request), _response(src._response)
{}

Client& Client::operator=(const Client &src)
{
	if (this != &src) {
		this->_request = src._request;
		this->_response = src._response;
	}
	return (*this);
} */

Client::~Client()
{
/* 	if (is_open(_socket))
		close(_socket); */
}

void Client::readRequest(const std::string &rawData)
{
	(void)rawData;
	//appeler parser Alex
}

void Client::processRequest(std::map<std::string, Location>& location, int size)
{
	(void)location;
	(void)size;
	//appeler gestion Baptiste
}

void Client::sendResponse()
{
	std::string response = _response.toString();
	//creer une fonction dans httpreponse pour tout mettre en une string
	const char* data = response.c_str();
	size_t total = response.size();
	size_t sent = 0;
	ssize_t n;

	while (sent < total)
	{
		n = send(_socket, data + sent, total - sent, 0);
		if (n == -1)
			throw std::runtime_error("Error sending reponse.");
		sent += n;
	}
}

void Client::setHttpRequest(const HttpRequest& request)
{
	_request = request;
}
void Client::setHttpResponse(const HttpResponse& response)
{
	_response = response;
}
		
HttpRequest& Client::getHttpRequest()
{
	return (_request);
}
HttpResponse& Client::getHttpResponse()
{
	return (_response);
}
int	Client::getClientSocket()
{
	return (_socket);
}

std::string Client::getIPaddress()
{
	char* cString = inet_ntoa(_address.sin_addr);
	std::string ipAddress = std::string(cString);
	return (ipAddress);
	//to implemant but probably without using inet_ntoa()
}
