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

// GET METHOD
void RequestController::handleGetResponse(const HttpRequest& req, HttpResponse& res) {
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();

    if (getResourceDatabase().find(uri) != getResourceDatabase().end()) {
        res.generate200OK("text/plain", "Resource found: " + uri);
    } else {
        res.generate404NotFound();
    }

    res.setHTTPVersion(version);
    res.ensureContentLength();
}

// POST METHOD
void RequestController::handlePostResponse(const HttpRequest& req, HttpResponse& res) {
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();
    std::string body = req.getBody();

    if (body.empty()) {
        res.generate400BadRequest("Bad Request: Empty body");
    } else {
        getResourceDatabase()[uri] = body;
        res.generate201Created(uri);
    }

    res.setHTTPVersion(version); 
    res.ensureContentLength();
}

// DELETE METHOD
void RequestController::handleDeleteResponse(const HttpRequest& req, HttpResponse& res) {
    std::string uri = req.getURI();
    std::string version = req.getHTTPVersion();

    if (getResourceDatabase().find(uri) != getResourceDatabase().end()) {
        getResourceDatabase().erase(uri);
        res.generate200OK("text/plain", "Resource deleted: " + uri);
    } else {
        res.generate404NotFound();
    }

    res.setHTTPVersion(version);
    // TO DO ensure content length?
}

// HANDLE METHODS FUNCTIONS (GET, POST, DELETE)
void GetRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    handleGetResponse(req, res);
}

void PostRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    handlePostResponse(req, res);
}

void DeleteRequestHandler::handle(const HttpRequest& req, HttpResponse& res) {
    handleDeleteResponse(req, res);
}

// DESTRUCTORS

GetRequestHandler::~GetRequestHandler() {
    // Implémentation vide ou nécessaire selon les besoins
}

PostRequestHandler::~PostRequestHandler() {
    // Implémentation vide ou nécessaire selon les besoins
}

DeleteRequestHandler::~DeleteRequestHandler() {
    // Implémentation vide ou nécessaire selon les besoins
}



