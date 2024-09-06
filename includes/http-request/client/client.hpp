#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <iostream>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

// Représente une connexion client, gérant les requêtes HTTP,
// les réponses, et l'état de la connexion.

class Client {
    private : 
        int _fd;
        std::string _ipAddress;
        HttpRequest _request;
        HttpResponse _response;

    public:
        Client(int fd, const std::string& ipAdress);
        ~Client()
        Client(const Client & src);
        Client& operator=(const Client & src);
        void readRequest() const;
        void sendResponse() const;
        void closeConnection() const;
};

#endif