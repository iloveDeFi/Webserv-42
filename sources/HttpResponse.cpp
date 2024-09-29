#include "../includes/HttpResponse.hpp"
#include "../includes/HttpException.hpp"


// Constructeur par défaut
HttpResponse::HttpResponse()
    : _version("HTTP/1.1"), _statusCode(200), _statusMessage("OK"), _headers(), _body(""), _isChunked(false) {}

HttpResponse::~HttpResponse() {}

HttpResponse::HttpResponse(const HttpResponse& src)
    : _version(src._version), _statusCode(src._statusCode), _statusMessage(src._statusMessage),
      _headers(src._headers), _body(src._body), _isChunked(src._isChunked) {}

HttpResponse& HttpResponse::operator=(const HttpResponse& src) {
    if (this != &src) {
        _version = src._version;
        _statusCode = src._statusCode;
        _statusMessage = src._statusMessage;
        _headers = src._headers;
        _body = src._body;
        _isChunked = src._isChunked;
    }
    return *this;
}

// Getters
std::string HttpResponse::getHTTPVersion() const { return _version; }
int HttpResponse::getStatusCode() const { return _statusCode; }
std::string HttpResponse::getStatusMessage() const { return _statusMessage; }
std::map<std::string, std::string> HttpResponse::getHeaders() const { return _headers; }
std::string HttpResponse::getHeader(const std::string& name) const {
    std::map<std::string, std::string>::const_iterator it = _headers.find(name);
    return (it != _headers.end()) ? it->second : "";
}
std::string HttpResponse::getBody() const { return _body; }
bool HttpResponse::isChunked() const { return _isChunked; }

// Setters
void HttpResponse::setHTTPVersion(const std::string& version) {
    if (version != "HTTP/1.1" && version != "HTTP/1.0") {
        throw HttpException("Invalid HTTP version: " + version);
    }
    _version = version;
}

void HttpResponse::setStatusCode(int statusCode) {
    if (statusCode < 100 || statusCode > 599) {
        std::stringstream ss;
        ss << statusCode;  // Convertir statusCode en chaîne
        throw HttpException("Invalid status code: " + ss.str());
    }
    _statusCode = statusCode;  // Assignation de statusCode valide
}


void HttpResponse::setStatusMessage(const std::string& statusMessage) {
    if (statusMessage.empty()) {
        throw HttpException("Status message cannot be empty");
    }
    _statusMessage = statusMessage;
}

void HttpResponse::setHeaders(const std::map<std::string, std::string>& headers) { _headers = headers; }
void HttpResponse::setBody(const std::string& body) { _body = body; }
void HttpResponse::setIsChunked(bool isChunked) { _isChunked = isChunked; }

// Parsing principal
void HttpResponse::parse(const std::string& raw_response) {
    if (!isValidResponse(raw_response)) {
        throw HttpException("Invalid response format: response is not properly formatted.");
    }

    try {
        setHTTPVersion(extractHTTPVersion(raw_response));
        setStatusCode(extractStatusCode(raw_response));
        setStatusMessage(extractStatusMessage(raw_response));
        setHeaders(extractHeaders(raw_response));
        setBody(extractBody(raw_response));
        setIsChunked(checkIfChunked(_headers));

		if (isChunked()) {
			setBody(decodeChunkedBody(getBody()));
        }
    } catch (const HttpException& e) {
        std::cerr << "Error during response parsing: " << e.what() << std::endl;
        clearResponseData();
    }
}

// Fonctions auxiliaires
std::string HttpResponse::safe_substr(const std::string& str, size_t start, size_t length) {
    if (start >= str.size()) throw HttpException("Substring extraction failed: start index out of bounds.");
    if (start + length > str.size()) throw HttpException("Substring extraction failed: length out of bounds.");
    return str.substr(start, length);
}

std::string HttpResponse::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return safe_substr(str, first, last - first + 1);
}

bool HttpResponse::isValidResponse(const std::string& raw_response) {
    size_t version_end = raw_response.find(' ');
    size_t status_end = raw_response.find(' ', version_end + 1);
    size_t message_end = raw_response.find("\r\n", status_end + 1);
    if (version_end == std::string::npos || status_end == std::string::npos || message_end == std::string::npos) {
        return false;
    }
    return true;
}

// Extraction des composants
std::string HttpResponse::extractHTTPVersion(const std::string& raw_response) {
    size_t version_end = raw_response.find(' ');
    if (version_end == std::string::npos) throw MissingHTTPVersionException();
    return safe_substr(raw_response, 0, version_end);
}

int HttpResponse::extractStatusCode(const std::string& raw_response) {
    size_t version_end = raw_response.find(' ');
    size_t status_end = raw_response.find(' ', version_end + 1);
    if (status_end == std::string::npos) throw HttpException("Missing status code.");
    return atoi(raw_response.substr(version_end + 1, status_end - version_end - 1).c_str());
}

std::string HttpResponse::extractStatusMessage(const std::string& raw_response) {
    size_t status_end = raw_response.find(' ', raw_response.find(' ') + 1);
    size_t message_end = raw_response.find("\r\n", status_end + 1);
    if (message_end == std::string::npos) throw HttpException("Missing status message.");
    return safe_substr(raw_response, status_end + 1, message_end - status_end - 1);
}

std::map<std::string, std::string> HttpResponse::extractHeaders(const std::string& raw_response) {
    std::map<std::string, std::string> headers;
    size_t headers_start = raw_response.find("\r\n") + 2;
    size_t headers_end = raw_response.find("\r\n\r\n", headers_start);
    if (headers_end == std::string::npos) throw HttpException("Missing end of headers.");

    size_t current = headers_start;
    while (current < headers_end) {
        size_t line_end = raw_response.find("\r\n", current);
        if (line_end == std::string::npos || line_end > headers_end) break;

        size_t colon_pos = raw_response.find(':', current);
        if (colon_pos == std::string::npos || colon_pos > line_end) {
            throw HttpException("Invalid header format: missing ':' in header.");
        }

        std::string key = safe_substr(raw_response, current, colon_pos - current);
        std::string value = safe_substr(raw_response, colon_pos + 1, line_end - colon_pos - 1);
        headers[trim(key)] = trim(value);

        current = line_end + 2;
    }

    return headers;
}

std::string HttpResponse::extractBody(const std::string& raw_response) {
    size_t body_start = raw_response.find("\r\n\r\n") + 4;
    return safe_substr(raw_response, body_start, raw_response.size() - body_start);
}

bool HttpResponse::checkIfChunked(const std::map<std::string, std::string>& headers) {
    std::map<std::string, std::string>::const_iterator it = headers.find("Transfer-Encoding");
    if (it != headers.end() && it->second == "chunked") {
        return true;
    }
    return false;
}

// Gestion du transfert chunked
std::string HttpResponse::decodeChunkedBody(const std::string& rawBody) {
    std::string decodedBody;
    std::istringstream iss(rawBody);
    std::string line;
    size_t totalSize = 0;

    while (std::getline(iss, line)) {
        if (!line.empty() && line[line.length()-1] == '\r') {
            line.erase(line.length()-1);
        }

        std::string::size_type semicolonPos = line.find(';');
        if (semicolonPos != std::string::npos) {
            line = line.substr(0, semicolonPos);
        }

        unsigned long chunkSize;
        char* endPtr;
        chunkSize = strtoul(line.c_str(), &endPtr, 16);
        if (*endPtr != '\0') {
            throw HttpException("Invalid chunk size");
        }

        if (chunkSize == 0) break;

        if (totalSize + chunkSize > MAX_BODY_SIZE) {
            throw HttpException("Body size exceeds limit");
        }

        char* chunk = new char[chunkSize];
        iss.read(chunk, static_cast<std::streamsize>(chunkSize));
        if (iss.gcount() != static_cast<std::streamsize>(chunkSize)) {
            delete[] chunk;
            throw HttpException("Unexpected end of chunk");
        }

        decodedBody.append(chunk, chunkSize);
        delete[] chunk;
        totalSize += chunkSize;

        iss.ignore(2);
    }

    // Traiter les trailers si nécessaire

    return decodedBody;
}

void HttpResponse::clearResponseData() {
    _version = "";
    _statusCode = 0;
    _statusMessage = "";
    _headers.clear();
    _body = "";
    _isChunked = false;
}

//N'inclut pas de logique pour créer des réponses spécifiques (comme les listings de répertoires ou les erreurs).