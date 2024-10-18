#include "CgiHandler.hpp"
#include "MngmtServers.hpp"

CgiHandler::CgiHandler(const std::string& scriptPath)
:_scriptPath(scriptPath)
{}

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

void CgiHandler::setEnv(HttpRequest request, std::string ip, const _server &serverInfo)
{
    std::map<std::string, std::string> env;
    std::string method = request.getMethod();
    env["REQUEST_METHOD"] = method;

    if (method == "GET")
        env["QUERY_STRING"] = request.getQueryParameters();
    else
    {
        env["CONTENT_TYPE"] = request.getHeader("Content-Type");
        env["CONTENT_LENGTH"] = request.getHeader("Content-Length");
    }
    env["SCRIPT_FILENAME"] = request.getURI();
    env["REMOTE_ADDR"] = ip;
    env["SERVER_NAME"] = serverInfo._name;
    env["SERVER_PORT"] = to_string_c98(serverInfo._port);
    env["HTTP_USER_AGENT"] = request.getHeader("User-Agent");
    env["HTTP_COOKIE"] = request.getHeader("Cookie");   

    std::vector<std::string> envVar;
    for (std::map<std::string, std::string>::const_iterator it = env.begin(); \
    it != env.end(); it++)
	{
		std::string tmp = it->first + "=" + it->second;
		envVar.push_back(tmp);
	}
    _envVars = envVar;
}

void CgiHandler::handle(HttpResponse &res)
{
    res.setBody(execCgi(res));

    if (res.getStatusCode() != 200)
        return ;
    res.setHeader("Content-Type", "text/plain");
    res.ensureContentLength();
}

std::string CgiHandler::execCgi(HttpResponse &res)
{
    int pipefd[2];
    pid_t pid;
    char **argv = NULL;
    int status;
    std::vector<const char*> envp;

    if (pipe(pipefd) == -1)
        res.generate500InternalServerError("pipe failed");

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
            res.generate500InternalServerError("execve failed");
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
        res.generate500InternalServerError("fork failed");
    return (NULL);
}

