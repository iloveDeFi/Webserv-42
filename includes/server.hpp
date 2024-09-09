
#ifndef SERVER_HPP
# define SERVER_HPP
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

class Server
{
	private:
		Socket* _serverSocket;
		/* std::vector<std::string> parseIndex(const std::string& value);
		std::pair<int, std::string> parseError(const std::string& value); */
		std::string _name;
		int	_ipAddress;
		int	_port;
		int _maxSize; 
		std::map<std::string, Location> _locations;
		
	public:
		Server(const std::string& fdConfig, const std::map<std::string, Location>& location);
		/* Server(const Server &other) = delete;
		Server& operator=(const Server &other) = delete; */
		~Server();

		void loadConfig(const std::string& configFilePath);
		Location& parseLocation(std::ifstream config, std::string& value, std::string key);
		void handleRequest();
		void prepareFdSets(fd_set &readFds, \
		const std::vector<int> &clientFds, int &maxFd);
		void acceptNewClients(std::vector<int> &clientFds/* , int &maxFd */);
		void handleActiveClients(fd_set &readFds, std::vector<int> &clientFds);
		void handleClient(int clientSocket, int ip);
		std::string readRawData(int clientSocket);

		int getPort();
		int	getSize();
		std::map<std::string, Location>& getLocation();

		void setIpAddress(int ip);

};

#endif
