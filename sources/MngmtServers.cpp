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
		newServer._root = server.root;

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
	std::vector<Client> clients;
	int maxFd = 0;

	while (true)
	{
		prepareFdSets(readFds, clients, maxFd);

		// Attendre 5 secondes (et 0microscd) pour un événement sur les
		// sockets surveillés avant de retourner.
		// timeout permet de contrôler la fréquence à laquelle
		// le serveur traite les entrées sans être bloqué indéfiniment
		// en attente d'activité
		struct timeval tv = {5, 0};
		int selectRes = select(maxFd + 1, &readFds, NULL, NULL, &tv);
		if (selectRes > 0)
		{
			acceptNewClients(clients, readFds);
			handleActiveClients(readFds, clients);
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
									std::vector<Client> &clients, int &maxFd)
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
    for (size_t i = 0; i < clients.size(); ++i)
    {
        int clientFd = clients[i].getClientSocket();
        FD_SET(clientFd, &readFds);
        if (clientFd > maxFd)
            maxFd = clientFd;
    }
}

// Si une connexion est repérée, elle doit être acceptée avant de pourvoir
// être traiter. accept() créé un socket() chez le client,
// afin de pouvoir être non bloquant cette socket aussi doit être en non bloquant
// FD_ISSET() returns true if the file descFD_CLOEXECriptor fd is a
// member of the set pointed to by fdset. (int FD_ISSET(int fd, fd_set *fdset))
void ManagementServer::acceptNewClients(std::vector<Client> &clients, fd_set &readFds)
{
    for (size_t i = 0; i < _servers.size(); i++)
    {
        if (FD_ISSET(_servers[i]._serverSocket->getFdSocket(), &readFds))
        {
            std::cout << "Connection detected on server " << _servers[i]._name << std::endl;
            struct sockaddr_in clientAddr;
            try
            {
                int clientFd = _servers[i]._serverSocket->Accept(clientAddr);
                setNonBlocking(clientFd);
                Client newClient(clientFd, clientAddr);
                clients.push_back(newClient);
                std::cout << "New client connected on server " << _servers[i]._name << ": " << clientFd << std::endl;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error during client initialization: " << e.what() << std::endl;
            }
        }
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
}


// Loop sur tout les fd actifs pour les gérer individuellement
// si une erreur arrive, le client est supprimé des clients actifs
void ManagementServer::handleActiveClients(fd_set &readFds, std::vector<Client> &clients)
{
    for (size_t i = 0; i < clients.size(); ++i)
    {
        int clientFd = clients[i].getClientSocket();
        if (FD_ISSET(clientFd, &readFds))
        {
            try
            {
                handleClient(clients[i]);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Client error: " << e.what() << std::endl;
                FD_CLR(clientFd, &readFds);
                close(clientFd);
                clients.erase(clients.begin() + i);
                --i;
            }
        }
    }
}


void ManagementServer::handleClient(Client &client)
{
    int clientSocket = client.getClientSocket();
    struct sockaddr_in serverAddr;
    socklen_t serverAddrLen = sizeof(serverAddr);

    //détermine sur quel serveur le client est connecté
    if (getsockname(clientSocket, (struct sockaddr *)&serverAddr, &serverAddrLen) == -1)
    {
        throw std::runtime_error("Error getting server socket information: " + std::string(strerror(errno)));
    }

    int serverPort = ntohs(serverAddr.sin_port);

    //trouver la conf du serveur correspondant à ce port
    _server currentServer;
    bool serverFound = false;
    for (std::vector<_server>::iterator it = _servers.begin(); it != _servers.end(); ++it)
    {
        if (it->_port == serverPort)
        {
            currentServer = *it;
            serverFound = true;
            break;
        }
    }
    if (!serverFound)
    {
        throw std::runtime_error("No server found for port " + std::to_string(serverPort));
    }

    // Lire la requête du client
    std::string rawData = readRawData(clientSocket);
	
    if (rawData.empty())
    {
        // Si aucune donnée n'a été lue, le client a peut-être fermé la connexion
        throw std::runtime_error("No data received from client.");
    }
	// TO DO : delete?
	client.readRequest(rawData); // parser renvoyé à Alex
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
    size_t headerEndPos = std::string::npos;
    size_t contentLength = 0;

    // Read headers
    while (true)
    {
        bytesReceived = recv(clientSocket, buffer, buffer_size - 1, 0);
        if (bytesReceived > 0)
        {
            buffer[bytesReceived] = '\0';
            requestData.append(buffer, bytesReceived);

            // Look for the end of the headers
            headerEndPos = requestData.find("\r\n\r\n");
            if (headerEndPos != std::string::npos)
                break;
        }
        else if (bytesReceived == 0) // Client closed the connection
            return "";
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            else
                throw std::runtime_error("Error reading from socket: " + std::string(strerror(errno)));
        }
    }

    // Parse headers to find Content-Length
    std::string headers = requestData.substr(0, headerEndPos + 2); // Include \r\n
    std::istringstream headerStream(headers);
    std::string line;
    while (std::getline(headerStream, line))
    {
        if (!line.empty() && line.back() == '\r') // Remove \r
            line.pop_back();

        if (line.empty())
            break; // End of headers

        if (line.find("Content-Length:") != std::string::npos)
        {
            std::string value = line.substr(line.find(":") + 1);
            contentLength = std::stoi(value);
        }
    }

    // Read the body based on Content-Length
    size_t totalBytesToRead = headerEndPos + 4 + contentLength;
    while (requestData.size() < totalBytesToRead)
    {
        bytesReceived = recv(clientSocket, buffer, buffer_size - 1, 0);
        if (bytesReceived > 0)
        {
            buffer[bytesReceived] = '\0';
            requestData.append(buffer, bytesReceived);
        }
        else if (bytesReceived == 0) // Client closed the connection
            break;
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            else
                throw std::runtime_error("Error reading from socket: " + std::string(strerror(errno)));
        }
    }
	//std::cout << "HERE!!!!! " << requestData << std::endl;
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
