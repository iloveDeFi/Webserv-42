#pragma once
#ifndef MNGMTSERVERS_HPP
# define MNGMTSERVERS_HPP
# include <iostream>
# include <vector>
# include <map>
# include <netinet/in.h>
# include <sys/socket.h>
# include <fstream>
# include <sstream>
# include <utility>
# include <string>
# include <poll.h>
# include "Socket.hpp"
# include "Utils.hpp"
# include "HttpConfig.hpp"
# include "HttpController.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"

struct _server
{
	Socket* _serverSocket;
	std::string _name;
	int	_ipAddress;
	int	_port;
	int _maxSize;
	std::map<int, std::string> _errorPages;
	std::vector<HttpConfig::Location> _locations;
};

# include "Client.hpp"

class ManagementServer
{
	private:
		 std::vector<_server> _servers;
		 void addNewServer(HttpConfig::ServerConfig server);
		
	public:
		ManagementServer(HttpConfig &config);
		/* Server(const Server &other) = delete;
		Server& operator=(const Server &other) = delete; */
		~ManagementServer();
		void setNonBlocking(int fd);
		Location& parseLocation(std::ifstream config, std::string& value, std::string key);
		void handleRequest();
		void prepareFdSets(fd_set &readFds, \
		const std::vector<int> &clientFds, int &maxFd);
		void acceptNewClients(std::vector<int> &clientFds, fd_set &readFds);
		void handleActiveClients(fd_set &readFds, std::vector<int> &clientFds);
		void handleClient(int clientSocket);
		std::string readRawData(int clientSocket);

		int getPort(std::vector<_server>::iterator it);
		int	getSize(std::vector<_server>::iterator it);
		_server& getServerInfo(std::vector<_server>::iterator it);

		void setIpAddress(std::vector<_server>::iterator it, int ip);

};

#endif