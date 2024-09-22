#pragma once

#include <stdexcept>
#include <string>


class HttpException : public std::runtime_error {
public:
    explicit HttpException(const std::string& message)
        : std::runtime_error(message) {}
};

class MissingBodyException : public HttpException {
public:
    MissingBodyException()
        : HttpException("Invalid request format: missing body.") {}
};

class InvalidQueryStringException : public HttpException {
public:
    InvalidQueryStringException()
        : HttpException("Invalid request format: invalid query string.") {}
};


class MissingMethodException : public HttpException {
public:
    MissingMethodException()
        : HttpException("Invalid request format: missing method.") {}
};

class MissingURIException : public HttpException {
public:
    MissingURIException()
        : HttpException("Invalid request format: missing URI.") {}
};

class MissingHTTPVersionException : public HttpException {
public:
    MissingHTTPVersionException()
        : HttpException("Invalid request format: missing HTTP version.") {}
};

class MissingHeadersException : public HttpException {
public:
    MissingHeadersException()
        : HttpException("Invalid request format: missing headers.") {}
};

class MissingEndOfHeadersException : public HttpException {
public:
    MissingEndOfHeadersException()
        : HttpException("Invalid request format: missing end of headers.") {}
};

class InvalidHeaderFormatException : public HttpException {
public:
    InvalidHeaderFormatException()
        : HttpException("Invalid header format: missing colon or invalid header.") {}
};

class BodyTooLargeException : public HttpException {
public:
    BodyTooLargeException() : HttpException("Request body is too large.") {}
};