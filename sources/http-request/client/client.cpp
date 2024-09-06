#include "client.hpp"

Client::Client(int fd, std::string& ipAdress) : _fd(fd), _ipAddress(ipAdress) {}

Client::~Client() {}

Client::Client(const Client & src) : _ipAddress(src._ipAddress), _request(src._request), _response(src._response) {

}

Client& Client::operator=(const Client & src) {
    if (this != &src) {
        this->_ipAddress = src._ipAddress;
        this->_request = src._request;
        this->_response = src._response;
    }
    return (*this);
}