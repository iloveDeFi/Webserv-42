
#include "Socket.hpp"

Socket::Socket(int domain, int service, int protocol, int port, u_long interface)
{
	std::cout << "port in constructor socket " << port << std::endl;
	_fdSocket = socket(domain, service, protocol);
	if (_fdSocket < 0)
		throw std::runtime_error("Error creating socket.");
	
	_address.sin_family = domain;
	_address.sin_addr.s_addr = interface;
	_address.sin_port = htons(port);

	_len = sizeof(_address);
}

/* Socket::Socket(const Socket &other)
:_fdSocket(other._fdSocket), _address(other._address), _len(other._len)
{}
Socket& Socket::operator=(const Socket &other)
{
	if (this != &other)
	{
		_fdSocket = other._fdSocket;
		_address = other._address;
		_len = other._len;
	}
	return (*this);
} */

Socket::~Socket()
{
	//if (is_open(_fdSocket))
		close(_fdSocket);
}

void Socket::Bind()
{
	if (bind(_fdSocket, (struct sockaddr*)&_address, _len) < 0)
		throw std::runtime_error("Error binding socket.");
	std::cout << "Binding complete on port " << ntohs(_address.sin_port) << std::endl;
}

void Socket::Listen()
{
	if (listen(_fdSocket, MAX_CONNECTIONS) < 0)
		throw std::runtime_error("Error listening socket.");
	std::cout << "Listening complete" << std::endl;
}

int Socket::Accept(struct sockaddr_in &clientAddr)
{
    socklen_t clientLen = sizeof(clientAddr);
    int clientSocket = accept(_fdSocket, (struct sockaddr*)&clientAddr, &clientLen);
    if (clientSocket < 0)
        throw std::runtime_error("Error accepting socket.");
	_clientAddr = clientAddr;
    return clientSocket;
}


ssize_t Socket::Send(int client_socket, const char* buffer, size_t buffer_length, int flags)
{
	ssize_t tmp = send(client_socket, buffer, buffer_length, flags);
	if (tmp < 0)
		throw std::runtime_error("Error sending socket.");
	return (tmp);
}

ssize_t Socket::Receive(int client_socket, char* buffer, size_t buffer_length, int flags)
{
	ssize_t tmp = recv(client_socket, buffer, buffer_length, flags);
	if (tmp < 0)
		throw std::runtime_error("Error receiving socket.");
	return (tmp);
}

int Socket::getFdSocket()
{
	return(_fdSocket);
}
struct sockaddr_in &Socket::getAddress()
{
	return(this->_address);
}
socklen_t Socket::getLen()
{
	return(_len);
}



