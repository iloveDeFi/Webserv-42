#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include <iostream>
#include <map>

// Gère l'analyse et la représentation d'une requête HTTP.
class HttpRequest {
    private:
        std::string _method;
        std::string _uri;
        std::string _version;
        std::map<std::string, std::string> _headers;
        std::string _body;
        bool _is_chunked;

    public:
        HttpRequest();
        ~HttpRequest();
        HttpRequest(const HttpRequest& src);
        HttpRequest& operator=(const HttpRequest& src);

         // Getters
        std::string getMethod() const { return _method; }
        std::string getURI() const { return _uri; }
        std::string getHTTPVersion() const { return _version; }
        std::map<std::string, std::string> getHeaders() const { return _headers; }
        std::string getBody() const { return _body; }
        bool isChunked() const { return _is_chunked; }

        // Setter (si nécessaire pour simuler des données)
        void setMethod(const std::string& method) { _method = method; }
        void setURI(const std::string& uri) { _uri = uri; }
        void setHTTPVersion(const std::string& version) { _version = version; }
        void setHeaders(const std::map<std::string, std::string>& headers) { _headers = headers; }
        void setBody(const std::string& body) { _body = body; }
        void setIsChunked(bool is_chunked) { _is_chunked = is_chunked; }

        std::string getHeader(const std::string& name) const;
        void parse(const std::string& rawRequest);
};

#endif
