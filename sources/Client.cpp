
#include "Client.hpp"

Client::Client(int fd): _socket(fd) {}

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
	close(_socket);
}

void Client::readRequest(const std::string &rawData)
{
	//appeler parser Alex
}

void Client::processRequest()
{
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
