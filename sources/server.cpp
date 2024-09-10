#include "Server.hpp"

Server::Server(const std::string& fdConfig, const std::map<std::string, Location>& location) : _serverSocket(NULL)
{
	loadConfig(fdConfig);
	_locations = location;
	_serverSocket = new Socket(AF_INET, SOCK_STREAM, 0, _port, INADDR_ANY);
	socklen_t addrLen = sizeof(_serverSocket->_address);
	if (getsockname(_serverSocket->getFdSocket(), (struct sockaddr *)&_serverSocket->_address, &addrLen) == -1)
		throw std::runtime_error("Error getting host IP.");
	std::string ip = inet_ntoa(_serverSocket->_address.sin_addr); // Converting IP to string
	setIpAddress(ip);
	_serverSocket->Bind();
	_serverSocket->Listen();
}

Server::~Server()
{
	close(_serverSocket->getFdSocket());
	delete _serverSocket;
}

void Server::loadConfig(const std::string& configFilePath)
{
	std::string line;
	std::string key;
	std::string value;
	
	std::ifstream config(configFilePath.c_str());
	if (!config) throw std::runtime_error("Error opening configuration file.");
	
	while (getline(config, line))
	{
		std::istringstream lineStream(line);
		getline(lineStream, key, ':');
		getline(lineStream, value);
		
		if (key == "listen")
			_port = stoi(value);
		else if (key == "name")
			_name = value; // Changed from key to value
		else if (key == "size")
			_maxSize = stoi(value);
		else if (key == "location")
		{
			// Handle location parsing if required
		}
	}
}

void Server::handleRequest()
{
	fd_set readFds;
	std::vector<int> clientFds;
	int maxFd = _serverSocket->getFdSocket();

	while (true)
	{
		prepareFdSets(readFds, clientFds, maxFd);

		struct timeval tv = {5, 0};
		int selectRes = select(maxFd + 1, &readFds, NULL, NULL, &tv);

		if (selectRes == -1)
			throw std::runtime_error("Select error");
		else if (selectRes == 0)
		{
			std::cout << "Timeout occurred, performing routine checks." << std::endl;
			continue;
		}

		if (FD_ISSET(_serverSocket->getFdSocket(), &readFds))
			acceptNewClients(clientFds, maxFd);

		handleActiveClients(readFds, clientFds);
	}
}

void Server::prepareFdSets(fd_set &readFds, const std::vector<int> &clientFds, int &maxFd)
{
	FD_ZERO(&readFds);
	FD_SET(_serverSocket->getFdSocket(), &readFds);

	for (int clientFd : clientFds)
	{
		FD_SET(clientFd, &readFds);
		if (clientFd > maxFd)
			maxFd = clientFd;
	}
}

void Server::acceptNewClients(const std::vector<int> &clientFds, int &maxFd)
{
	int clientFd = _serverSocket->Accept();
	if (clientFd != -1)
	{
		clientFds.push_back(clientFd);
		std::cout << "New client connected: " << clientFd << std::endl;
	}
}

void Server::handleActiveClients(fd_set &readFds, const std::vector<int> &clientFds)
{
	for (size_t i = 0; i < clientFds.size(); ++i)
	{
		if (FD_ISSET(clientFds[i], &readFds))
		{
			try
				handleClient(clientFds[i]);
			catch (const std::exception& e)
			{
				std::cerr << "Client error: " << e.what() << std::endl;
				FD_CLR(clientFds[i], &readFds);
				close(clientFds[i]);
				clientFds.erase(clientFds.begin() + i);
				--i;
			}
		}
	}
}

void Server::handleClient(int clientSocket)
{
	Client client(clientSocket);
	std::string rawRequest = readRawData(clientSocket);
	client.readRequest(rawRequest);
	client.processRequest(getLocation(), getSize());
	client.sendResponse();
}

std::string Server::readRawData(int clientSocket)
{
	const size_t buffer_size = 1024;
	char buffer[buffer_size];
	std::string requestData;
	ssize_t bytesReceived;

	while ((bytesReceived = recv(clientSocket, buffer, buffer_size - 1, 0)) > 0)
	{
		buffer[bytesReceived] = '\0';
		requestData.append(buffer);
		if (requestData.find("\r\n\r\n") != std::string::npos)
			break;
	}
	if (bytesReceived < 0)
		throw std::runtime_error("Error reading from socket");
	
	return requestData;
}

int Server::getPort() const { return _port; }

int Server::getSize() const { return _maxSize; }

Location& Server::getLocation() { return _locations.front(); } // Adjust if needed

void Server::setIpAddress(const std::string& ip)
{
	_ipAddress = ip;
}

Location* Server::matchLocation(const std::string& requestUri)
{
	Location* bestMatch = nullptr;

	for (Location& loc : _locations)
	{
		if (requestUri.find(loc.uri) == 0)
		{
			if (!bestMatch || loc.uri.length() > bestMatch->uri.length())
			{
				bestMatch = &loc;
			}
		}
	}
	return bestMatch;
}
