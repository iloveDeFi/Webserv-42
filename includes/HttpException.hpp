#pragma once
#include "Logger.hpp"
#include <stdexcept>
#include <string>

// --- Exceptions HttpRequest ---

class HttpException : public std::runtime_error
{
public:
    explicit HttpException(const std::string &message)
        : std::runtime_error(message) {}
};

class MissingBodyException : public HttpException
{
public:
    MissingBodyException()
        : HttpException("Invalid request format: missing body.") {}
};

class InvalidQueryStringException : public HttpException
{
public:
    InvalidQueryStringException()
        : HttpException("Invalid request format: invalid query string.") {}
};

class MissingMethodException : public HttpException
{
public:
    MissingMethodException()
        : HttpException("Invalid request format: missing method.") {}
};

class MissingURIException : public HttpException
{
public:
    MissingURIException()
        : HttpException("Invalid request format: missing URI.") {}
};

class MissingHTTPVersionException : public HttpException
{
public:
    MissingHTTPVersionException()
        : HttpException("Invalid request format: missing HTTP version.") {}
};

class MissingHeadersException : public HttpException
{
public:
    MissingHeadersException()
        : HttpException("Invalid request format: missing headers.") {}
};

class MissingEndOfHeadersException : public HttpException
{
public:
    MissingEndOfHeadersException()
        : HttpException("Invalid request format: missing end of headers.") {}
};

class InvalidHeaderFormatException : public HttpException
{
public:
    InvalidHeaderFormatException()
        : HttpException("Invalid header format: missing colon or invalid header.") {}
};

class BodyTooLargeException : public HttpException
{
public:
    BodyTooLargeException() : HttpException("Request body is too large.") {}
};

// --- Exceptions HttpResponse ---

class HttpResponseException : public std::runtime_error
{
public:
    explicit HttpResponseException(const std::string &message)
        : std::runtime_error(message) {}
};

class InvalidHTTPVersionException : public HttpResponseException
{
public:
    InvalidHTTPVersionException()
        : HttpResponseException("Invalid response format: invalid HTTP version.") {}
};

class InvalidStatusCodeException : public HttpResponseException
{
public:
    InvalidStatusCodeException()
        : HttpResponseException("Invalid response format: invalid status code.") {}
};

class InvalidStatusMessageException : public HttpResponseException
{
public:
    InvalidStatusMessageException()
        : HttpResponseException("Invalid response format: invalid status message.") {}
};

class MissingResponseHeadersException : public HttpResponseException
{
public:
    MissingResponseHeadersException()
        : HttpResponseException("Invalid response format: missing headers.") {}
};

class MissingResponseBodyException : public HttpResponseException
{
public:
    MissingResponseBodyException()
        : HttpResponseException("Invalid response format: missing body.") {}
};

class InvalidChunkedEncodingException : public HttpResponseException
{
public:
    InvalidChunkedEncodingException()
        : HttpResponseException("Invalid response format: chunked transfer encoding error.") {}
};
