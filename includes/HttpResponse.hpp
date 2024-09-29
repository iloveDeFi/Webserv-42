#pragma once

#include "HttpMessage.hpp"
#include <stdexcept>
#include <string>
#include <sstream>
#include <cstdlib>

class HttpResponse {
private:
    std::string _version;
    int _statusCode;
    std::string _statusMessage;
    std::map<std::string, std::string> _headers;
    std::string _body;
    bool _isChunked;

public:
    // Constructeurs
    HttpResponse();
    ~HttpResponse();
    HttpResponse(const HttpResponse& src);
    HttpResponse& operator=(const HttpResponse& src);

    // Getters et Setters
    std::string getHTTPVersion() const;
    int getStatusCode() const;
    std::string getStatusMessage() const;
    std::map<std::string, std::string> getHeaders() const;
    std::string getHeader(const std::string& name) const;
    std::string getBody() const;
    bool isChunked() const;

    void setHTTPVersion(const std::string& version);
    void setStatusCode(int statusCode);
    void setStatusMessage(const std::string& statusMessage);
    void setHeaders(const std::map<std::string, std::string>& headers);
    void setBody(const std::string& body);
    void setIsChunked(bool isChunked);

    // Parsing et validation
    void parse(const std::string& raw_response);

    // Fonctions auxiliaires
    static std::string safe_substr(const std::string& str, size_t start, size_t length);
    static std::string trim(const std::string& str);
    static bool isValidResponse(const std::string& raw_response);

    // Extraction des composants de la réponse
    static std::string extractHTTPVersion(const std::string& raw_response);
    static int extractStatusCode(const std::string& raw_response);
    static std::string extractStatusMessage(const std::string& raw_response);
    static std::map<std::string, std::string> extractHeaders(const std::string& raw_response);
    static std::string extractBody(const std::string& raw_response);

    // Gestion du chunked transfer-encoding
    static bool checkIfChunked(const std::map<std::string, std::string>& headers);

    // Effacer les données de la réponse en cas d'erreur
    void clearResponseData();
};
