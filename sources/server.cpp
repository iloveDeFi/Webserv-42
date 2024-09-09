
#include "Server.hpp"

//const int PORT = 8888;

Server::Server(const std::string& fdConfig, \
const std::map<std::string, Location>& location):_serverSocket(NULL), _locations(location)
{
	int ip;
	socklen_t addrLen;

	loadConfig(fdConfig);
	_serverSocket = new Socket(AF_INET, SOCK_STREAM, 0, _port, INADDR_ANY);
	addrLen = sizeof(_serverSocket->getAddress());
	ip = getsockname(_serverSocket->getFdSocket(), \
	(struct sockaddr *)&_serverSocket->getAddress(), &addrLen);
	if (ip == -1)
		throw std::runtime_error("Error getting host IP.");
	setIpAddress(ip);
	_serverSocket->Bind();
	_serverSocket->Listen();
}

Server::~Server()
{
	close(_serverSocket->getFdSocket());
	delete _serverSocket;
}

//fichier recu imaginé :
//root:/html
// index:index.html,index.htm
// error:404,not_found.html
// error:500,error.html
// listing:true
// name:mywebserv
// listen:8888
//AJOUTER # Limitations:client_max_body_size 8M; ??
// ou location /images ??

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
			_name = key;
		else if (key == "size")
			_maxSize = stoi(value);// trouver moyen pour avoir que le chiffre (pas 1M)
/* 		else if (key == "location")
		{
			_location[value] = parselocation(config, value);
			
		} */
	}
}

/* Location& server::parseLocation(std::ifstream config, std::string& value, , std::string& key)
{
	
} */

void Server::handleRequest()
{
	fd_set readFds;
	std::vector<int> clientFds;
	int maxFd = _serverSocket->getFdSocket();

	while (true)
	{
		prepareFdSets(readFds, clientFds, maxFd);

		// Attendre 5 secondes (et 0microscd) pour un événement sur les 
		//sockets surveillés avant de retourner.
		//timeout permet de contrôler la fréquence à laquelle 
		//le serveur traite les entrées sans être bloqué indéfiniment 
		//en attente d'activité
		struct timeval tv = {5, 0};
		int selectRes = select(maxFd + 1, &readFds, NULL, NULL, &tv);

		if (selectRes == -1)
			throw std::runtime_error("Select error");
		else if (selectRes == 0)
		{
			std::cout << "Timeout occurred, performing routine checks." << std::endl;
			continue;
		}
		//FD_ISSET() returns true if the file descriptor fd is a 
		//member of the set pointed to by fdset. (int FD_ISSET(int fd, fd_set *fdset))
		if (FD_ISSET(_serverSocket->getFdSocket(), &readFds))
			acceptNewClients(clientFds/* , maxFd */);
		handleActiveClients(readFds, clientFds);
	}

}

void Server::prepareFdSets(fd_set &readFds, \
const std::vector<int> &clientFds, int &maxFd)
{
	//FD_ZERO() initializes the set pointed to by fdset to be empty.
	//FD_SET() adds the file descriptor fd to the set pointed to by fdset.
	FD_ZERO(&readFds);
	FD_SET(_serverSocket->getFdSocket(), &readFds);

	for (size_t i = 0; i < clientFds.size(); ++i)
	{
		FD_SET(clientFds[i], &readFds);
		if (clientFds[i] > maxFd)
			maxFd = clientFds[i];
	}
}
void Server::acceptNewClients(std::vector<int> &clientFds/* , int &maxFd */)
{
	int clientFd = _serverSocket->Accept();
	if (clientFd != -1)
	{
		clientFds.push_back(clientFd);
		std::cout << "New client connected: " << clientFd << std::endl;
	}
}
void Server::handleActiveClients(fd_set &readFds, std::vector<int> &clientFds)
{
	for (size_t i = 0; i < clientFds.size(); ++i)
	{
		if (FD_ISSET(clientFds[i], &readFds))
		{
			try
			{
				handleClient(clientFds[i], getpeername(clientFds[i], ));
			}
			catch (const std::exception& e)
			{
				std::cerr << "Client error: " << e.what() << std::endl;
				//FD_CLR removes the socket from the readFds set, 
				//so select() will no longer monitor it.
				FD_CLR(clientFds[i], &readFds);
				close(clientFds[i]);
				clientFds.erase(clientFds.begin() + i);
				--i;
			}
		}
	}
}

void Server::handleClient(int clientSocket, int ip)
{
	Client client(clientSocket, ip);

	client.readRequest(readRawData(clientSocket));// parser renvoyé à Alex
	//il ajoute a client sont attribut _request;
	client.processRequest(getLocation(), getSize()); //gestion de la requete par Baptiste
	//il ajoute a client sont attribut _response;
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

int Server::getPort(){return (_port);}

int Server::getSize(){return (_maxSize);}

std::map<std::string, Location>& Server::getLocation(){return (_locations);} 

void Server::setIpAddress(int ip)
{
	_ipAddress = ip;
}

/* std::pair<int, std::string> server::parseError(const std::string& value)
{
	std::istringstream valueStream(value);
	std::string errorCodeStr, errorFile;
	
	getline(valueStream, errorCodeStr, ',');
	getline(valueStream, errorFile);
	int errorCode = stoi(errorCodeStr);
	return std::make_pair(errorCode, errorFile);
}

std::vector<std::string> server::parseIndex(const std::string& value)
{
	std::vector<std::string> indices;
	std::istringstream valueStream(value);
	std::string index;
	
	while (std::getline(valueStream, index, ','))
		indices.push_back(index);
	return indices;
} */