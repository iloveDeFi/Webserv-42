#ifndef UTILS_HPP
#define UTILS_HPP
#include <iostream>
#include "Logger.hpp"

#define MAX_CONNECTIONS 500

typedef enum
{
	GET,
	POST,
	DELETE,
	UNKNOWN,
} httpRequestMethod;

typedef enum
{
	OK = 200,
	CREATED = 201,
	ACCEPTED = 202,
	MOVED_PERM = 301,
	TMP_REDIRECT = 307,
	PERM_REDIRECT = 308,
	BAD_REQUEST = 400,
	UNAUTHORIZED = 401,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	TOO_LARGE = 413,
	METHODE_NOT_ALLOWED = 405,
	INTERNAL_SERVER_ERROR = 500,
	NOT_IMPLEMENTED = 501
} HttpStatusCode;
#endif