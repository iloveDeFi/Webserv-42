
#ifndef SERVER_HPP
# define SERVER_HPP
# include <iostream>
# include <vector>
# include <map>
# include "Socket.hpp"
# include "Client.hpp"
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

class server
{
	private:
		Socket* _serverSocket;
		/* std::vector<std::string> parseIndex(const std::string& value);
		std::pair<int, std::string> parseError(const std::string& value); */
		std::string _name;
		int	_port;
		int _maxSize;
		//std::map<std::string, LocationConfig> _locations;
		
	public:
		server(const std::string& fdConfig);
		server(const server &other) = delete;
		server& operator=(const server &other) = delete;
		~server();

		int getPort();
		void loadConfig(const std::string& configFilePath);
		void handleRequest();
		void handleClient(int clientSocket);
		std::string readRawData(int clientSocket);

		void setServerSocket();
};

#endif
