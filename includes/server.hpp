#ifndef SERVER_HPP
# define SERVER_HPP
# include <iostream>
# include <vector>
# include <map>
# include "Socket.hpp"
# include "Client.hpp"
# include "Utils.hpp"
# include "Location.hpp"
# include "HttpRequest.hpp"
# include <netinet/in.h>
# include <sys/socket.h>
# include <fstream>
# include <sstream>
# include <utility>
# include <string>
# include <poll.h>

class Server
{
	private:
		Socket* _serverSocket;
		std::string _name;
		int _port;
		int _maxSize;
		std::vector<Location> _locations; 
		
	public:
		Server(const std::string& fdConfig, const std::map<std::string, Location>& location);
		Server(const Server &other) = delete;
		Server& operator=(const Server &other) = delete;
		~Server();

		void loadConfig(const std::string& configFilePath);
		Location& parseLocation(std::ifstream& config, std::string& value, std::string key);
		void handleRequest();
		void prepareFdSets(fd_set &readFds, const std::vector<int> &clientFds, int &maxFd);
		void acceptNewClients(const std::vector<int> &clientFds, int &maxFd);
		void handleActiveClients(fd_set &readFds, const std::vector<int> &clientFds);
		void handleClient(int clientSocket);
		std::string readRawData(int clientSocket);

		int getPort() const;
		int getSize() const;

		Location& getLocation();
		void addLocation(const Location& loc);
		Location* matchLocation(const std::string& requestUri);

		void setIpAddress(const std::string& ip);

};

#endif
