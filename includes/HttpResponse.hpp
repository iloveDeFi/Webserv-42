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
        std::string _version;
        int _statusCode;
        std::string _reasonMessage;
        // HEADER
        std::string _name;
        std::string _value;
        std::map<std::string, std::string> _headers;
        // BODY
        std::string _body;
        bool _isChunked;

    public:
        HttpResponse();
        ~HttpResponse();
        HttpResponse(const HttpResponse& src);
        HttpResponse& operator=(const HttpResponse& src);

        // Setters
        // 1) RESPONSE LINE
        void setHTTPVersion(const std::string& version);
        void setStatusCode(int status);
        void setReasonMessage(const std::string& reasonMessage);

        // 2) HEADER
        void setHeader(const std::string& name, const std::string& value);
        void setHeaders(const std::map<std::string, std::string>& headers);
        std::string addHeader(std::string allow, std::string method) const;

        // 3) BODY
        void setBody(const std::string& body);
        void setIsChunked(bool isChunked);

        // OTHER
        void ensureContentLength();
        std::string toString() const;

        // SPECIAL RESPONSES
        std::string generate404Error(const std::string& uri);
        std::string generateRedirection(const std::string& newUri);

        // PRINT DATA (Internal to class cause no getters in response)
        std::ostream& print(std::ostream& os) const;
};

#endif
