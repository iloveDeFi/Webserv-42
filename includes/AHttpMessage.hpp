#pragma once

#include <iostream>
#include <map>
#include <string>
#include "HttpException.hpp"

class AHttpMessage {
protected:
    std::string _version;
    std::map<std::string, std::string> _headers;
    std::string _body;
    bool _isChunked;

    // Méthodes auxiliaires communes
    std::string trim(const std::string& str);
    std::string safe_substr(const std::string& str, size_t start, size_t length);
    std::map<std::string, std::string> extractHeaders(const std::string& raw_message);
    std::string extractBody(const std::string& raw_message);
    bool checkIfChunked(const std::string& raw_message);
    void clearMessageData();

public:
    AHttpMessage();
    virtual ~AHttpMessage();
    AHttpMessage(const AHttpMessage& src);
    AHttpMessage& operator=(const AHttpMessage& src);

    // Getters
    std::string getVersion() const;
    std::map<std::string, std::string> getHeaders() const;
    std::string getHeader(const std::string& name) const;
    std::string getBody() const;
    bool isChunked() const;

    // Setters
    void setVersion(const std::string& version);
    void setHeaders(const std::map<std::string, std::string>& headers);
    void setBody(const std::string& body);
    void setIsChunked(bool isChunked);

    // Méthode virtuelle pure pour le parsing
    virtual void parse(const std::string& raw_message) = 0;
};
