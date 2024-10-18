#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP
# include <iostream>
# include <unistd.h>
# include <map>
# include <string>
# include <HttpRequest.hpp>
# include <HttpResponse.hpp>
# include <sstream>

template <typename T>
std::string to_string_c98(T value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

struct _server;

class CgiHandler
{
    private:
        std::string _scriptPath;
        std::vector<std::string> _envVars;
        std::string execCgi(HttpResponse &res);

    public:
        CgiHandler(const std::string& scriptPath);
        ~CgiHandler();
        void setEnv(HttpRequest request, std::string ip, const _server &serverInfo);
        virtual void handle(HttpResponse &res);
};

#endif