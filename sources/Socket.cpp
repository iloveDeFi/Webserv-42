
#include "Socket.hpp"

Socket::Socket(int domain, int service, int protocol, int port, u_long interface)
{
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
	if (is_open(_fdSocket))
		Close();
}

void Socket::Bind()
{
	if (bind(_fdSocket, (struct sockaddr*)&_address, _len) < 0)
		throw std::runtime_error("Error binding socket.");
}

void Socket::Listen()
{
	if (listen(_fdSocket, MAX_CONNECTIONS) < 0)
		throw std::runtime_error("Error listening socket.");
}

int Socket::Accept()
{
	if (accept(_fdSocket, (struct sockaddr*)&_address, &_len) < 0)
		throw std::runtime_error("Error accepting socket.");
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

// FD_CLOEXEC : Ce flag est utilisé pour indiquer que 
//le descripteur de fichier doit être automatiquement fermé 
//lors de l'exécution de la fonction exec(), 
//qui est utilisée pour lancer une nouvelle image de programme. 
//Cela empêche les nouveaux programmes exécutés de hériter ce descripteur de fichier.

// F_SETFL: est une commande utilisée avec fcntl 
//pour définir les flags du descripteur de fichier.
// Permet la modification dynamique des propriétés de descripteurs de fichiers. 

void Socket::SetNonBlocking(bool is_non_blocking)
{
	int flags;

	if (is_non_blocking)
		flags = O_NONBLOCK | FD_CLOEXEC;
	else
		flags = FD_CLOEXEC;
	if (fcntl(_fdSocket, F_SETFL, flags) == -1)
		throw std::runtime_error("Failed to configure socket flags");
}

void Socket::Close()
{
	close(_fdSocket);
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

/* const char *Socket::ErrorSocket::what() const throw()
{
	return "Error creating socket.";
}

const char *Socket::ErrorSendingSocket::what() const throw()
{
	return "Error sending data.";
}

const char *Socket::ErrorReceivingSocket::what() const throw()
{
	return "Error receiving data.";
} */

