#ifndef MNGMTSERVER_HPP
# define MNGMTSERVER_HPP
# include <iostream>
# include <vector>
# include <map>
# include "Socket.hpp"
# include "Client.hpp"
# include "Utils.hpp"
# include "Location.hpp"
# include <netinet/in.h>
# include <sys/socket.h>
# include <fstream>
# include <sstream>
# include <utility>
# include <string>
# include <poll.h>

/* struct LocationConfig
{
	std::string root = "/html";
	std::vector<std::string> index;
	std::map<int, std::string> errorPages;
	bool allowListing;

	LocationConfig(std::string r = "", bool listing = false) : root(r), allowListing(listing) {}
}; */

class ManagementServer
{
	private:
		struct _server
		{
			Socket* _serverSocket;
			/* std::vector<std::string> parseIndex(const std::string& value);
			std::pair<int, std::string> parseError(const std::string& value); */
			std::string _name;
			int	_ipAddress;
			int	_port;
			int _maxSize; 
			std::map<std::string, Location> _locations;
		};
		 std::vector<_server> _servers;
		 void addNewServer(std::istringstream &configLine, \
		 std::map<std::string, Location>& _locations);
		
	public:
		ManagementServer(std::ifstream& fdConfig, std::vector<std::map<std::string, Location> > serverLocations);
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
		std::map<std::string, Location>& getLocation(std::vector<_server>::iterator it);

		void setIpAddress(std::vector<_server>::iterator it, int ip);

};

#endif