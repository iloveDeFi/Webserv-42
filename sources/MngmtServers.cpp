#include "MngmtServers.hpp"

// fichier fdConfig imaginé sans retour à la ligne à part pour différents serveurs:
// server1: root:/html index:index.html,index.htm error:404,not_found.html
// error:500,error.html listing:true name:mywebserv listen:8888
// Limitations:client_max_body_size 8M; ??
// server2:root:/html index:index.html etc.

// note il faut qu'on trouve un moyen pour pouvoir passer
// la valeur de la taille des fichiers ()
//  • k or K: Kilobytes
//  • m or M: Megabytes
//  • g or G: Gigabytes

// constructeur du gestionnaire de serveurs
// il va découper le fichier en ligne (chacune représentant un serveur)
// et ajouter un serveur au veteur de _servers
ManagementServer::ManagementServer(HttpConfig &config)
{
	std::string line;

	std::vector<HttpConfig::ServerConfig>::iterator it = config.getParsedServers().begin();

	while (it != config.getParsedServers().end())
	{
		addNewServer(*it);
		it++;
	}
}

ManagementServer::~ManagementServer()
{
	for (std::vector<_server>::iterator it = _servers.begin();
		 it != _servers.end(); it++)
	{
		close(it->_serverSocket->getFdSocket());
		delete it->_serverSocket;
		// delete la map location ?
	}
}

// suite du constructeur
//  cette fonction remplie en gros toute la struct du serveur
//  et active l'écoute passive du port
//  En détail:
// parse la ligne entre key et value pour récupérer les infos
// de config importantes
//  crée une Socket (classe), la définie comme non-bloquante
//  la lie avec le port d'écoute (bind) et écoute les requests (listen)
#include <sstream>

void ManagementServer::addNewServer(HttpConfig::ServerConfig server)
{
	_server newServer;
	socklen_t addrLen;

	try
	{
		newServer._name = server.serverName;
		newServer._port = server.port;
		newServer._maxSize = server.clientMaxBodySize;
		newServer._errorPages = server.errorPages;
		newServer._locations = server.locations;

		newServer._serverSocket = new Socket(AF_INET, SOCK_STREAM, 0, newServer._port, INADDR_ANY);
		addrLen = sizeof(newServer._serverSocket->getAddress());

		setNonBlocking(newServer._serverSocket->getFdSocket());

		try
		{
			newServer._serverSocket->Bind();
		}
		catch (const std::runtime_error &e)
		{
			std::ostringstream errorMsg;
			if (errno == EACCES)
			{
				errorMsg << "Permission denied. You may need root privileges to bind to port " << newServer._port;
				throw std::runtime_error(errorMsg.str());
			}
			else if (errno == EADDRINUSE)
			{
				errorMsg << "Address already in use. Port " << newServer._port << " may already be occupied.";
				throw std::runtime_error(errorMsg.str());
			}
			else
			{
				throw; // Rethrow the original exception if it's not one of the specific cases we're handling
			}
		}

		newServer._serverSocket->Listen();

		int ip = getsockname(newServer._serverSocket->getFdSocket(),
							 (struct sockaddr *)&newServer._serverSocket->getAddress(),
							 &addrLen);
		if (ip == -1)
		{
			std::string errorStr = "Error getting host IP: ";
			errorStr += strerror(errno);
			throw std::runtime_error(errorStr);
		}

		_servers.push_back(newServer);
		_servers.back()._ipAddress = ip;
		std::cout << "Server is listening on port " << newServer._port << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Failed to set up server on port " << server.port << ": " << e.what() << std::endl;

		// Clean up resources if an error occurred
		if (newServer._serverSocket)
		{
			delete newServer._serverSocket;
		}

		// Optionally, you might want to rethrow the exception or handle it in some other way
		// throw;
	}
}
// La boucle principale d'écoute des différents serveur lancés
// prépare les FD des serveurs et des potentiels clients
// en modifiant la valeur max du nombre d'FD actif
// commence par 4 (1, 2 et 3 étant les fd réservés) + 1 pour
// le fd(socket) du serveur
// select() (similaire à poll) permet de faire le I/O multiplexing
// en récupéreant les fd préalablement préparés
// si des nouvelles connections sont repérées, elles sont
// acceptées puis gérées
void ManagementServer::handleRequest()
{
	fd_set readFds;
	std::vector<int> clientFds;
	int maxFd = 0;

	while (true)
	{
		prepareFdSets(readFds, clientFds, maxFd);

		// Attendre 5 secondes (et 0microscd) pour un événement sur les
		// sockets surveillés avant de retourner.
		// timeout permet de contrôler la fréquence à laquelle
		// le serveur traite les entrées sans être bloqué indéfiniment
		// en attente d'activité
		struct timeval tv = {5, 0};
		int selectRes = select(maxFd + 1, &readFds, NULL, NULL, &tv);
		if (selectRes > 0)
		{
			acceptNewClients(clientFds, readFds);
			handleActiveClients(readFds, clientFds);
		}
		else if (selectRes == -1)
			throw std::runtime_error("Select error");
		else if (selectRes == 0)
		{
			std::cout << "Timeout occurred, performing routine checks." << std::endl;
			continue;
		}
		std::cout << "select() returned: " << selectRes << std::endl;
	}
}

// fonction permettant de préparer et initialiser les fd
//  Typically, the fd_set data type is implemented as a bit mask.
// However, we don’t need to know the details, since all manipulation
//  of file descriptor sets is done via four macros:
// FD_ZERO(), FD_SET(), FD_CLR(), and FD_ISSET().
// FD_ZERO() initializes the set pointed to by fdset to be empty.
// FD_SET() adds the file descriptor fd to the set pointed to by fdset.
void ManagementServer::prepareFdSets(fd_set &readFds,
									 const std::vector<int> &clientFds, int &maxFd)
{

	FD_ZERO(&readFds);
	for (size_t i = 0; i < _servers.size(); i++)
	{
		int serverFd = _servers[i]._serverSocket->getFdSocket();
		FD_SET(serverFd, &readFds);
		if (serverFd > maxFd)
			maxFd = serverFd;
		std::cout << "server Maxfd " << maxFd << std::endl;
	}
	for (size_t i = 0; i < clientFds.size(); ++i)
	{
		FD_SET(clientFds[i], &readFds);
		if (clientFds[i] > maxFd)
			maxFd = clientFds[i];
		std::cout << "client Maxfd " << maxFd << std::endl;
	}
}

// Si une connexion est repérée, elle doit être acceptée avant de pourvoir
// être traiter. accept() créé un socket() chez le client,
// afin de pouvoir être non bloquant cette socket aussi doit être en non bloquant
// FD_ISSET() returns true if the file descFD_CLOEXECriptor fd is a
// member of the set pointed to by fdset. (int FD_ISSET(int fd, fd_set *fdset))
void ManagementServer::acceptNewClients(std::vector<int> &clientFds, fd_set &readFds)
{
	for (size_t i = 0; i < _servers.size(); i++)
	{
		if (FD_ISSET(_servers[i]._serverSocket->getFdSocket(), &readFds))
		{
			std::cout << "Connection detected on server " << _servers[i]._name << std::endl;
			int clientFd = _servers[i]._serverSocket->Accept();
			if (clientFd != -1)
			{
				setNonBlocking(clientFd);
				clientFds.push_back(clientFd);
				std::cout << "New client connected on server " << _servers[i]._name << ": " << clientFd << std::endl;
			}
		}
		else
			std::cerr << "Error accepting client connection: " << strerror(errno) << std::endl;
	}
}

// Configure la socket en non bloquant
//  FD_CLOEXEC : Ce flag est utilisé pour indiquer que
// le descripteur de fichier doit être automatiquement fermé
// lors de l'exécution de la fonction exec(),
// qui est utilisée pour lancer une nouvelle image de programme.
// Cela empêche les nouveaux programmes exécutés de hériter ce descripteur de fichier.
//  F_SETFL: est une commande utilisée avec fcntl
// pour définir les flags du descripteur de fichier.
//  Permet la modification dynamique des propriétés de descripteurs de fichiers.
void ManagementServer::setNonBlocking(int fd)
{
<<<<<<< HEAD
	int flags;

	// Set O_NONBLOCK
	if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
		throw std::runtime_error("Failed to get file flags");
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("Failed to set non-blocking mode");

	// Set FD_CLOEXEC
	if ((flags = fcntl(fd, F_GETFD)) == -1)
		throw std::runtime_error("Failed to get fd flags");
	if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
		throw std::runtime_error("Failed to set FD_CLOEXEC");
=======
    int flags;

    // Set O_NONBLOCK
    if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
        throw std::runtime_error("Failed to get file flags");
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error("Failed to set non-blocking mode");

    // Set FD_CLOEXEC
    if ((flags = fcntl(fd, F_GETFD)) == -1)
        throw std::runtime_error("Failed to get fd flags");
    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
        throw std::runtime_error("Failed to set FD_CLOEXEC");
>>>>>>> develop
}


// Loop sur tout les fd actifs pour les gérer individuellement
// si une erreur arrive, le client est supprimé des clients actifs
void ManagementServer::handleActiveClients(fd_set &readFds, std::vector<int> &clientFds)
{
	for (size_t i = 0; i < clientFds.size(); ++i)
	{
		if (FD_ISSET(clientFds[i], &readFds))
		{
			try
			{
				handleClient(clientFds[i]);
			}
			catch (const std::exception &e)
			{
				std::cerr << "Client error: " << e.what() << std::endl;
				// FD_CLR removes the socket from the readFds set,
				// so select() will no longer monitor it.
				FD_CLR(clientFds[i], &readFds);
				close(clientFds[i]);
				clientFds.erase(clientFds.begin() + i);
				--i;
			}
		}
	}
}

// getpeername() et getsockname() ne servent qu'à
// construire la classe Client, à partir de laquelle
// on appelle le parsing de la request puis son traitement
// avant de renvoyer la response au client
void ManagementServer::handleClient(int clientSocket)
{
	struct sockaddr_in clientAddr;
	struct sockaddr_in serverAddr;
	socklen_t addrLen = sizeof(clientAddr);
	socklen_t serverAddrLen = sizeof(serverAddr);

	if (getpeername(clientSocket, (struct sockaddr *)&clientAddr, &addrLen) == -1)
		throw std::runtime_error("Error getting client IP address");

	// déterminer le serveur sur lequel est le client
	if (getsockname(clientSocket, (struct sockaddr *)&serverAddr, &serverAddrLen) == -1)
		throw std::runtime_error("Error getting server socket information");
	int serverPort = ntohs(serverAddr.sin_port);
	_server currentServer;
	for (std::vector<_server>::iterator it = _servers.begin(); it != _servers.end(); ++it)
	{
		if (it->_port == serverPort)
		{
			currentServer = *it;
			break;
		}
	}
	Client client(clientSocket, clientAddr);

	// TO DO : delete?
	client.readRequest(readRawData(clientSocket)); // parser renvoyé à Alex
	// il ajoute a client sont attribut _request;

	// TO CHECK : yes rawData values are GOOD here
	// std::string rawData = readRawData(clientSocket);
	// client.readRequest(rawData);
	// std::cout << "BEFORE PROCESS : Raw request data: " << rawData << std::endl;

	client.processRequest(currentServer); // gestion de la requete par Baptiste
	// il ajoute a client sont attribut _response;
	client.sendResponse();
}

std::string ManagementServer::readRawData(int clientSocket)
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

int ManagementServer::getPort(std::vector<_server>::iterator it)
{
	return (it->_port);
}

int ManagementServer::getSize(std::vector<_server>::iterator it)
{
	return (it->_maxSize);
}

/* _server& ManagementServer::\
getServerInfo(std::vector<_server>::iterator it)
{
	return (*it);
}  */

void ManagementServer::setIpAddress(std::vector<_server>::iterator it, int ip)
{
	it->_ipAddress = ip;
}
