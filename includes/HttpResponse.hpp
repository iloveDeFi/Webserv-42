#pragma once

#include "AHttpMessage.hpp"
#include <iostream>
#include <map>
#include <string>
#include <set>

class HttpResponse : public AHttpMessage {
private:
    int _statusCode;
    std::string _statusMessage;
    bool _isChunked;

public:
    HttpResponse();
    virtual ~HttpResponse();
    HttpResponse(const HttpResponse& src);
    HttpResponse& operator=(const HttpResponse& src);

    // Méthode héritée de AHttpMessage
    virtual void parse(const std::string& raw_response);
	virtual std::string toString() const;

    // 1) RESPONSE LINE
    std::set<std::string> initMethods() const;
    void setHTTPVersion(const std::string& httpVersion);
    void setStatusCode(int statusCode);

    // 2) HEADERS
    void setHeader(const std::string& name, const std::string& value);
    void addHeader(const std::string& name, const std::string& value);

    // 3) BODY
    void setBody(const std::string& body);
    void setIsChunked(bool isChunked);
    void ensureContentLength();

    // 4) TEMPLATE
    std::string generate404Error(const std::string& uri);
    std::string generateRedirection(const std::string& newUri);

    // 5) OTHERS
    std::string responseToString() const;

    // Getters
    int getStatusCode() const;
    std::string getStatusMessage() const;
	static std::string getStatusMessage(int statusCode);
    bool isChunked() const;
};

// Fonction pour imprimer les détails de la réponse
std::ostream& operator<<(std::ostream& os, const HttpResponse& res);
