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

struct ServerData;

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
    void processRequest(const ServerData &serverInfo, size_t maxSize);
    void sendResponse();

    void setHttpRequest(const HttpRequest &request);
    void setHttpResponse(const HttpResponse &response);

    HttpRequest &getHttpRequest();
    HttpResponse &getHttpResponse();

    int getClientSocket();
    std::string getIPaddress();
    bool isConnected() const;
    struct sockaddr_in &getClientAddr();
    bool checkFileExists(const std::string &filePath);
    bool isKeepAlive() const;
    bool handleRedirect(const ServerData &serverInfo, const std::string &method, HttpResponse &response);
    const HttpConfig::Location* findMatchingLocation(const ServerData &serverInfo, const std::string &uri);
    void handleRequest(const HttpRequest &request, const HttpConfig::Location &location, const ServerData &serverInfo, HttpResponse &response);

};

#endif