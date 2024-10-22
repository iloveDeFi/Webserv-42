
#ifndef SOCKET_HPP
#define SOCKET_HPP
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include "Utils.hpp"
#include "Logger.hpp"
class Socket
{
private:
	int _fdSocket;
	struct sockaddr_in _address;
	struct sockaddr_in _clientAddr;
	socklen_t _len;

public:
	Socket(int domain, int service, int protocol, int port, u_long interface);
	// toutes les classes qui gèrent des ressources
	// systèmes comme les sockets ne devraient pas pourvoir
	//  être copiés sans discernement. La copie d'une socket
	// peut entraîner des comportements non définis, comme
	// des fermetures de socket inattendues ou des
	// conflits sur les ressources réseau.
	/* Socket(const Socket &other) = delete;
	Socket& operator=(const Socket &other) = delete; */
	~Socket();

	void Bind();
	void Listen();
	int Accept(struct sockaddr_in &clientAddr);

	ssize_t Send(int client_socket, const char *buffer, size_t buffer_length, int flags);
	ssize_t Receive(int client_socket, char *buffer, size_t buffer_length, int flags);

	int getFdSocket();
	struct sockaddr_in &getAddress();
	socklen_t getLen();
};

#endif