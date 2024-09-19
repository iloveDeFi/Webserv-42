#include "HttpController.hpp"

// TO DO : need coplien's form if memory management needed ex. dynamic
static std::map<std::string, std::string> defaultResourceDatabase;

// Constructeur par défaut, utilise une référence vers la base de données statique par défaut
RequestController::RequestController()
: _resourceDatabase(defaultResourceDatabase)  // Liaison de la référence à une map statique
{
    _resourceDatabase[""] = "";  // Exemple de modification de la map statique
}

// Constructeur prenant une référence valide
/* RequestController::RequestController(std::map<std::string, std::string>& RsrDatabase)
: _resourceDatabase(RsrDatabase)  // Liaison à une référence valide passée par l'appelant
{
} */

// Destructeur (pas de gestion spéciale nécessaire ici)
RequestController::~RequestController() {}


RequestController::RequestController(const RequestController& src)
: _resourceDatabase(src._resourceDatabase)
{
		(void)src;
}

RequestController& RequestController::operator=(const RequestController& src)
{
	(void)src;

	return (*this);
}

void GetRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();
    std::string queryParams = req.getQueryParameters();
    //std::string cookies = req.getCookies(); //BONUS
 
    std::string responseBody = "You requested: " + uri + "\nQuery Params: " + queryParams + "\nCookies: "; // + cookies;

    res.setHTTPVersion(version);
    res.setStatusCode(200);
    res.setHeader("Content-Type", "text/plain");
    // TO DO : Or to set body use data base resource from server
    // res.setBody(resourceDatabase[path]);
    res.setBody(responseBody);
    res.ensureContentLength();
}

void PostRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();
    std::string body = req.getBody();
    //std::string cookies = req.getCookies();
    std::string queryParams = req.getQueryParameters(); 

    std::string responseBody = "Received POST data for: " + uri + "\nBody: " + body + "\nCookies: " /* + cookies */;

    res.setStatusCode(201);
    res.setBody(responseBody);
    res.setHTTPVersion(version);
    res.setHeader("Content-Type", "text/plain");
    res.ensureContentLength();
}

bool checkResourceExists(const std::string& uri, const std::map<std::string, std::string>& resourceDatabase) {
    return resourceDatabase.find(uri) != resourceDatabase.end();
}

bool deleteResource(const std::string& uri, std::map<std::string, std::string>& resourceDatabase) {
    std::map<std::string, std::string>::iterator it = resourceDatabase.find(uri);
    if (it != resourceDatabase.end()) {
        resourceDatabase.erase(it);
        return true;
    }
    return false;
}

void DeleteRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();

    bool resourceExists = checkResourceExists(uri, getResourceDatabase());
    std::string responseBody;

    if (resourceExists) {
        deleteResource(uri, getResourceDatabase());
        responseBody = "Resource deleted: " + uri;
        res.setStatusCode(200);
    } else {
        responseBody = "Resource not found: " + uri;
        res.setStatusCode(404);
    }

    res.setBody(responseBody);
    res.setHTTPVersion(version);
    res.setHeader("Content-Type", "text/plain");
    res.ensureContentLength();
}

GetRequestHandler::GetRequestHandler() {
    // Constructeur par défaut
}

PostRequestHandler::PostRequestHandler() {
    // Constructeur par défaut
}

DeleteRequestHandler::DeleteRequestHandler() {
    // Constructeur par défaut
}

GetRequestHandler::~GetRequestHandler() {
    // Implémentation vide ou nécessaire selon les besoins
}

PostRequestHandler::~PostRequestHandler() {
    // Implémentation vide ou nécessaire selon les besoins
}

DeleteRequestHandler::~DeleteRequestHandler() {
    // Implémentation vide ou nécessaire selon les besoins
}



