#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include "Logger.hpp"
class HttpResponse
{
private:
    std::string _httpVersion;
    int _statusCode;
    std::string _reasonMessage;
    std::map<std::string, std::string> _headers;
    std::string _body;
    bool _isChunked;

public:
    HttpResponse();
    ~HttpResponse();
    HttpResponse(const HttpResponse &src);
    HttpResponse &operator=(const HttpResponse &src);

    void generate200OK(const std::string &contentType, const std::string &bodyContent);
    void generate201Created(const std::string &location);
    void generate202Accepted(const std::string &deletionInfo);
    void generate204NoContent(const std::string &errorMessage);
    void generate400BadRequest(const std::string &errorMessage);
    void generate403Forbidden(const std::string &errorMessage);
    void generate404NotFound(const std::string &errorMessage);
    void generate405MethodNotAllowed(const std::string &allowedMethods);
    void generate409Conflict(const std::string &conflictInfo);
    void generate500InternalServerError(const std::string &errorMessage);
    void generate501NotImplemented(const std::string &errorMessage);
    std::string getFullResponse();

    void setHTTPVersion(const std::string &version);
    void setStatusCode(int status);
    void setReasonMessage(const std::string &reasonMessage);

    void setHeader(const std::string &name, const std::string &value);
    void setHeaders(const std::map<std::string, std::string> &headers);
    std::string addHeader(std::string allow, std::string method) const;
    void setContentType(std::string contentType);
    void setContentLength(int contentLength);
    void setBody(const std::string &body);
    void setIsChunked(bool isChunked);

    int getStatusCode() const;
    std::string normalizeHeader(const std::string &header);
    void ensureContentLength();
    std::string toString();
    std::string generate404Error(const std::string &uri);
    std::string generateRedirection(const std::string &newUri);
    std::ostream &print(std::ostream &os) const;

    void logHttpResponse(Logger &logger);
};

#endif
