
#include "server.hpp"

//const int PORT = 8888;

server::server(const std::string& fdConfig):_serverSocket(NULL)
{
	loadConfig(fdConfig);
	_serverSocket = new Socket(AF_INET, SOCK_STREAM, 0, _port, INADDR_ANY);
	_serverSocket->Bind();
	_serverSocket->Listen();
}

server::~server()
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
// port:8888
//AJOUTER # Limitations:client_max_body_size 8M; ??
// ou location /images ??

void server::loadConfig(const std::string& configFilePath)
{
	std::string line;
	std::string key;
	std::string value;
	
	std::ifstream config(configFilePath.c_str());
	if (!config) throw std::runtime_error("Error opening configuration file.");
	
	while (std::getline(config, line))
	{
		std::istringstream lineStream(line);
		std::getline(lineStream, key, ':');
		std::getline(lineStream, value);
		/* if (key == "root")
			_locations["default"].root = value;
		else if (key == "index")
			_locations["default"].index = parseIndex(value);
		else if (key == "error")
			_locations["default"].errorPages.insert(parseError(value));
		else if (key == "listing")
			_locations["default"].allowListing = (value == "true"); */
		if (key == "name")
			_name = key;
		else if (key == "port")
			_port = stoi(value);
		else if (key == "size")
			_maxSize = stoi(value);
	}
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

void server::handleRequest()
{
	fd_set readFds;
	struct timeval tv;
	int retval;

	while (true)
	{  
		FD_ZERO(&readFds);
		FD_SET(_serverSocket->getFdSocket(), &readFds);

		// Set timeout as needed
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		retval = select(_serverSocket->getFdSocket() + 1, &readFds, NULL, NULL, &tv);

		if (retval == -1)
			throw std::runtime_error("Select error");
		else if (retval)
		{
			if (FD_ISSET(_serverSocket->getFdSocket(), &readFds))
			{
				int clientFd = _serverSocket->Accept();
				if (clientFd != -1)
					handleClient(clientFd);  // Handle each client in a separate method or thread
			}
		} 
		else
		{
			// Handle timeout: perform any periodic checks or maintenance
		}
	}
}

void server::handleClient(int clientSocket)
{
	Client client(clientSocket);//comment avoir l'adresse ip?

	client.readRequest(readRawData(clientSocket));// parser renvoyé à Alex
	//il ajoute a client sont attribut _request;
	client.processRequest(); //gestion de la requete par Baptise
	//il ajoute a client sont attribut _response;

	// Renvoyer la _response au client
	client.sendResponse();
}

std::string server::readRawData(int clientSocket)
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
		perror("Error reading from socket");

	return requestData;
}
