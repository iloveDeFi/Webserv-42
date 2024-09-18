#pragma once

#include <stdexcept>
#include <string>


class HttpRequestException : public std::runtime_error {
public:
    explicit HttpRequestException(const std::string& message)
        : std::runtime_error(message) {}
};

class MissingBodyException : public HttpRequestException {
public:
    MissingBodyException()
        : HttpRequestException("Invalid request format: missing body.") {}
};

class InvalidQueryStringException : public HttpRequestException {
public:
    InvalidQueryStringException()
        : HttpRequestException("Invalid request format: invalid query string.") {}
};


class MissingMethodException : public HttpRequestException {
public:
    MissingMethodException()
        : HttpRequestException("Invalid request format: missing method.") {}
};

class MissingURIException : public HttpRequestException {
public:
    MissingURIException()
        : HttpRequestException("Invalid request format: missing URI.") {}
};

class MissingHTTPVersionException : public HttpRequestException {
public:
    MissingHTTPVersionException()
        : HttpRequestException("Invalid request format: missing HTTP version.") {}
};

class MissingHeadersException : public HttpRequestException {
public:
    MissingHeadersException()
        : HttpRequestException("Invalid request format: missing headers.") {}
};

class MissingEndOfHeadersException : public HttpRequestException {
public:
    MissingEndOfHeadersException()
        : HttpRequestException("Invalid request format: missing end of headers.") {}
};

class InvalidHeaderFormatException : public HttpRequestException {
public:
    InvalidHeaderFormatException()
        : HttpRequestException("Invalid header format: missing colon or invalid header.") {}
};

class BodyTooLargeException : public HttpRequestException {
public:
    BodyTooLargeException() : HttpRequestException("Request body is too large.") {}
};