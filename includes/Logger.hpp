#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <ctime>
#include <Templates.hpp>
class Logger
{
private:
    std::ofstream _logFile;
    Logger(const std::string &filename);
    Logger(const Logger &);
    Logger &operator=(const Logger &);

    /* 	•	Variable statique : Cela signifie que _instance est partagé entre toutes les instances de la classe Logger. Il n’est pas spécifique à une instance et est accessible à travers la classe elle-même.
    •	Pointeur : Le type de _instance est un pointeur vers Logger. Cela permet de différer l’initialisation de l’objet jusqu’à ce qu’il soit réellement nécessaire. Ainsi, _instance peut pointer vers un objet Logger ou rester nullptr tant que l’objet n’est pas créé.
*/

    static Logger *_instance;

public:
    static Logger &getInstance(const std::string &filename = "server.log");

    ~Logger();

    void log(const std::string &message);
    void logError(const std::string &errorMessage);
    void logRequest(const std::string &method, const std::string &uri, int statusCode);
    void logAccess(const std::string &path);

private:
    std::string filename;
    std::string getCurrentTime();
};

#endif