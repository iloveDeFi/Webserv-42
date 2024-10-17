#pragma once

#include <iostream>
#include <map>
#include <string>
#include "HttpException.hpp"


class HttpMessage {
protected:
    std::string _version;  // HTTP version (ex: "HTTP/1.1")
    std::map<std::string, std::string> _headers;  // Headers of the HTTP message
    std::string _body;  // Body of the HTTP message

public:
    // Constructors and Destructor
    HttpMessage();
    virtual ~HttpMessage();

    // Getters
    std::string getHTTPVersion() const;
    std::map<std::string, std::string> getHeaders() const;
    std::string getHeader(const std::string& name) const;
    std::string getBody() const;

    // Setters
    void setHTTPVersion(const std::string& version);
    void setHeaders(const std::map<std::string, std::string>& headers);
    void setHeader(const std::string& name, const std::string& value);
    void setBody(const std::string& body);

    // Utility
    std::string safe_substr(const std::string& str, size_t start, size_t length) const;
    std::string trim(const std::string& str) const;
};
