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


class CgiHandler
{
    private:
        std::string _scriptPath;
        std::vector<std::string> _envVars;
        std::string execCgi(HttpResponse &res);
        std::vector<std::string> setEnv(HttpRequest request, const std::string ip, int port);

    public:
        CgiHandler(const std::string& scriptPath,  std::vector<std::string> env);
        ~CgiHandler();
        
        virtual void handle(const HttpRequest &req, HttpResponse &res);
};

#endif