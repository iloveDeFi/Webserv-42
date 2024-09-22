#pragma once

#include "AHttpMessage.hpp"
#include <set>

class HttpRequest : public AHttpMessage {
private:
    std::string _method;
    std::string _uri;
    std::map<std::string, std::string> _queryParameters;
    std::set<std::string> _allowedMethods;

    static const std::set<std::string> initMethods();
    std::string extractMethod(const std::string& raw_request);
    std::string extractURI(const std::string& raw_request);
    std::map<std::string, std::string> extractQueryParameters(const std::string& uri);
    bool isValidRequest(const std::string& raw_request);

public:
    HttpRequest();
    ~HttpRequest();
    HttpRequest(const HttpRequest& src);
    HttpRequest& operator=(const HttpRequest& src);

    void parse(const std::string& raw_request);

    // Getters
    std::string getMethod() const;
    std::string getURI() const;
    std::map<std::string, std::string> getQueryParameters() const;

    // Setters
    void setMethod(const std::string& method);
    void setURI(const std::string& uri);
    void setQueryParameters(const std::map<std::string, std::string>& query_params);
};

// Fonction pour imprimer les détails de la requête
std::ostream& operator<<(std::ostream& os, const HttpRequest& req);