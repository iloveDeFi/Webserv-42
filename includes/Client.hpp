#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "HttpConfig.hpp"
#include "HttpController.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "MngmtServers.hpp"
#include "Logger.hpp"
#include "Templates.hpp"
#include <iostream>
#include <arpa/inet.h>
#include <sstream>
#include <cerrno>

struct _server;

class Logger;

class Client
{
private:
    int _socket;
    struct sockaddr_in _address;
    HttpRequest _request;
    HttpResponse _response;

public:
    Client(int socket, const struct sockaddr_in &address);
    ~Client();

    void readRequest(const std::string &rawData);
    void processRequest(const _server &serverInfo);
    void sendResponse();

    void setHttpRequest(const HttpRequest &request);
    void setHttpResponse(const HttpResponse &response);

    HttpRequest &getHttpRequest();
    HttpResponse &getHttpResponse();

    int getClientSocket();
    std::string getIPaddress();
    bool isConnected() const;
    struct sockaddr_in& getClientAddr();
};

#endif