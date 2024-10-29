#pragma once
#ifndef MNGMTSERVERS_HPP
#define MNGMTSERVERS_HPP

#include <iostream>
#include <vector>
#include <map>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fstream>
#include <sstream>
#include <utility>
#include <string>
#include <string.h>
#include <poll.h>
#include "Socket.hpp"
#include "Utils.hpp"
#include "HttpConfig.hpp"


class RequestController; 

struct ServerData
{
    Socket *ServerDataSocket;
    std::string _name;
    int _ipAddress;
    int _port;
    int _maxSize;
    std::map<int, std::string> _errorPages;
    std::vector<HttpConfig::Location> _locations;
    std::string _root;
};


#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Client.hpp"
#include "Logger.hpp"

class Client;
//class HttpRequest;
//class HttpResponse;

class ManagementServer
{
private:
	std::vector<ServerData> ServerDatas;
	void addNewServer(HttpConfig::ServerConfig server);

public:
	ManagementServer(HttpConfig &config);
	/* Server(const Server &other) = delete;
	Server& operator=(const Server &other) = delete; */
	~ManagementServer();
	void setNonBlocking(int fd);
	HttpConfig::Location &parseLocation(std::ifstream config, std::string &value, std::string key);
	void handleRequest();
	void prepareFdSets(fd_set &readFds,
					   std::vector<Client> &clients, int &maxFd);
	void acceptNewClients(std::vector<Client> &clients, fd_set &readFds);
	void handleActiveClients(fd_set &readFds, std::vector<Client> &clients);
	bool handleClient(Client &client);
	std::string readRawData(int clientSocket, size_t maxBodySize);

	int getPort(std::vector<ServerData>::iterator it);
	int getSize(std::vector<ServerData>::iterator it);
	ServerData &getServerInfo(std::vector<ServerData>::iterator it);

	void setIpAddress(std::vector<ServerData>::iterator it, int ip);

	class RequestTooLargeException : public std::exception
	{
		public:
			const char* what() const throw()
			{
				return "Request entity too large";
			}
	};
	
};



#endif