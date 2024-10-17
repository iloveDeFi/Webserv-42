#include "CgiHandler.hpp"

CgiHandler::CgiHandler(const std::string& scriptPath,  std::vector<std::string> env)
:_scriptPath(scriptPath), _envVars(env) {}

CgiHandler::~CgiHandler()
{
    std::vector<std::string>::iterator it = _envVars.begin();
    while (it != _envVars.end())
    {
        std::vector<std::string>::iterator tmp = it;
        it++;
        delete &tmp;
    }
}

std::vector<std::string> CgiHandler::setEnv(HttpRequest request, std::string ip, int port)
{
    std::map<std::string, std::string> env;
    env["REQUEST_METHOD"] = request.getMethod();
    env["SCRIPT_FILENAME"] = request.getURI();
    env["QUERY_STRING"] = request.getQueryParameters();
    env["CONTENT_TYPE"] = request.getHeader("Content-Type");
    env["CONTENT_LENGTH"] = request.getHeader("Content-Length");
    env["REMOTE_ADDR"] = ip;
    env["SERVER_NAME"] = "localhost";
    env["SERVER_PORT"] = to_string_c98(port);
    env["HTTP_USER_AGENT"] = request.getHeader("User-Agent");
    env["HTTP_COOKIE"] = request.getHeader("Cookie");   

    std::vector<std::string> envVar;
    for (std::map<std::string, std::string>::const_iterator it = env.begin(); \
    it != env.end(); it++)
	{
		std::string tmp = it->first + "=" + it->second;
		envVar.push_back(strdup((tmp.c_str())));
	}
    envVar.push_back(NULL);
    return (envVar);
}
void CgiHandler::handle(const HttpRequest &req, HttpResponse &res)
{
    res.setBody(execCgi(res));
    //a comprendre
    res.setHeader();
    res.ensureContentLength();
}

std::string CgiHandler::execCgi(HttpResponse &res)
{
    int pipefd[2];
    pid_t pid;
    char **argv;
    int status;
    std::vector<const char*> envp;

    if (pipe(pipefd) == -1)
    {
        res.setStatusCode(500);
        throw std::runtime_error("pipe failed");
    }

    argv[0] = (char *)_scriptPath.c_str();
    argv[1] = NULL;

    for (std::vector<std::string>::const_iterator it = _envVars.begin(); it != _envVars.end(); ++it)
        envp.push_back(it->c_str());
    envp.push_back(NULL);


    if ((pid = fork()) == 0)
    {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        if (execve(argv[0], argv, const_cast<char* const*>(envp.data())) == -1)
        {
            res.setStatusCode(500);
            throw std::runtime_error("execve failed");
        }
        exit(1); 
    }
    else if (pid > 0)
    {
        close(pipefd[1]);
        char buffer[1024];
        std::string result;
        ssize_t bytesRead;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            result.append(buffer, bytesRead);
        }
        close(pipefd[0]);
        waitpid(pid, &status, 0);
        res.setStatusCode(status);
        return (result);
    }
    else
    {
        res.setStatusCode(500);
        throw std::runtime_error("fork failed");
    }
}

