#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include "HttpRequestException.hpp"

const size_t MAX_BODY_SIZE = 2 * 1024 * 1024; // 2 MB

class HttpRequest {
private:
    std::string _method;
    std::string _uri;
    std::string _version;
    std::map<std::string, std::string> _headers;
    std::string _body;
    std::map<std::string, std::string> _queryParameters;
    bool _isChunked;
    std::set<std::string> _allowedMethods;

    static const std::set<std::string> initMethods();

    // Méthodes privées pour extraire les parties de la requête
    std::string extractMethod(const std::string& raw_request);
    std::string extractURI(const std::string& raw_request);
    std::map<std::string, std::string> extractHeaders(const std::string& raw_request);
    std::string extractBody(const std::string& raw_request);
    std::string extractHTTPVersion(const std::string& raw_request);
    std::map<std::string, std::string> extractQueryParameters(const std::string& uri);
    bool checkIfChunked(const std::string& raw_request);

    // Fonctions auxiliaires
    std::string trim(const std::string& str);
    std::string safe_substr(const std::string& str, size_t start, size_t length);
    bool hasBody(const std::string& raw_request);
    bool isValidRequest(const std::string& raw_request);
    void clearRequestData();
    static std::string toLower(const std::string& str);

public:
    HttpRequest();
    ~HttpRequest();
    HttpRequest(const HttpRequest& src);
    HttpRequest& operator=(const HttpRequest& src);

    // Méthode de parsing principale
    void parse(const std::string& raw_request);

    // Getters
    std::string getMethod() const;
    std::string getURI() const;
    std::string getHTTPVersion() const;
    std::map<std::string, std::string> getHeaders() const;
    std::string getHeader(const std::string& name) const;
    std::string getBody() const;
    std::map<std::string, std::string> getQueryParameters() const;
    std::string getQueryString() const;
    bool isChunked() const;

    // Setters
    void setMethod(const std::string& method);
    void setURI(const std::string& uri);
    void setHTTPVersion(const std::string& version);
    void setHeaders(const std::map<std::string, std::string>& headers);
    void setBody(const std::string& body);
    void setIsChunked(bool isChunked);
    void setQueryParameters(const std::map<std::string, std::string>& queryParameters);
};

// Fonction pour imprimer les détails de la requête
std::ostream& operator<<(std::ostream& os, const HttpRequest& req);

// Fonction de test
void testRequest(const std::string& raw_request);

#endif