#include <iostream>
#include <string>
#include "../includes/HttpResponse.hpp"

int main() {
    // Exemple de réponse HTTP brute
    std::string raw_response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: 138\r\n"
        "Connection: keep-alive\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "<html>\r\n"
        "<head><title>An Example Page</title></head>\r\n"
        "<body>Hello World, this is a very simple HTML document.</body>\r\n"
        "</html>";

    try {
        // Création d'un objet HttpResponse
        HttpResponse response;

        // Parsing de la réponse brute
        response.parse(raw_response);

        // Affichage des informations de la réponse
        std::cout << "HTTP Version: " << response.getHTTPVersion() << std::endl;
        std::cout << "Status Code: " << response.getStatusCode() << std::endl;
        std::cout << "Status Message: " << response.getStatusMessage() << std::endl;
        std::cout << "Content-Type: " << response.getHeader("Content-Type") << std::endl;
        std::cout << "Content-Length: " << response.getHeader("Content-Length") << std::endl;
        std::cout << "Connection: " << response.getHeader("Connection") << std::endl;
        std::cout << "Is Chunked: " << (response.isChunked() ? "Yes" : "No") << std::endl;
        std::cout << "Body: " << response.getBody() << std::endl;
    } catch (const HttpResponseException& e) {
        // Gestion des erreurs
        std::cerr << "Error while parsing the response: " << e.what() << std::endl;
    }

    return 0;
}
