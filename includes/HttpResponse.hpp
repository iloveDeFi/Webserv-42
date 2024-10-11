#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include <iostream>
#include <map>
#include <string>

// My response format is:
// 1) HTTP Response Line
// 2) HTTP Response Headers
// X) (empty line)
// 3) HTTP Response Body
class HttpResponse {
    private:
        // RESPONSE LINE
        std::string _httpVersion;
        int _statusCode;
        std::string _reasonMessage;
        // HEADERS
        std::map<std::string, std::string> _headers; // content-type, content-length
        // cache-control
        // expires
        // location
        // server
        // set-cookie
        // (separation lines)

        // BODY
        std::string _body;
        bool _isChunked;

    public:
        HttpResponse();
        ~HttpResponse();
        HttpResponse(const HttpResponse& src);
        HttpResponse& operator=(const HttpResponse& src);

        // Response
        void generate200OK(const std::string& contentType, const std::string& bodyContent); // 200 OK response
        void generate201Created(const std::string& location); // 201 Created response
        void generate202Accepted(const std::string& deletionInfo); // 202 Accepted response
        void generate204NoContent(); // 204 No Content response
        void generate403Forbidden(); // 403 Forbidden response
        void generate404NotFound(); // 404 Not Found response
        void generate409Conflict(const std::string& conflictInfo); // 409 Conflict response for PUT method
        void generate500InternalServerError(); // 500 Internal Server Error response
        std::string getFullResponse();

        // Setters
        // 1) RESPONSE LINE
        void setHTTPVersion(const std::string& version);
        void setStatusCode(int status);
        void setReasonMessage(const std::string& reasonMessage);

        // 2) HEADERS
        void setHeader(const std::string& name, const std::string& value);
        void setHeaders(const std::map<std::string, std::string>& headers);
        std::string addHeader(std::string allow, std::string method) const;
        void setContentType(std::string contentType);
        void setContentLength(int contentLength);
        // name
        // value
        // cache-control
        // expires
        // location
        // server
        // set-cookie

        // 3) BODY
        // TO DO : other types of data?
        void setBody(const std::string& body);
        void setIsChunked(bool isChunked);
        // (separation lines)

        // OTHER
        std::string normalizeHeader(const std::string& header);
        void ensureContentLength();
        std::string toString() const; // or use std::to_string ?

        // SPECIAL RESPONSES
        std::string generate404Error(const std::string& uri);
        std::string generateRedirection(const std::string& newUri);

        // PRINT DATA (Internal to class cause no getters in response)
        std::ostream& print(std::ostream& os) const;
};

#endif
