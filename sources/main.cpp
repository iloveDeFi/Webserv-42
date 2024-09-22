#include "../includes/AHttpMessage.hpp"
#include "../includes/HttpRequest.hpp"
#include "../includes/HttpResponse.hpp"
#include "../includes/HttpException.hpp"
#include <iostream>
#include <string>
#include <cassert>

void testHttpRequest() {
    std::cout << "Testing HttpRequest..." << std::endl;

    // Test valid request
    {
        std::string rawRequest = "GET /index.html HTTP/1.1\r\nHost: www.example.com\r\n\r\n";
        HttpRequest request;
        try {
            request.parse(rawRequest);
            assert(request.getMethod() == "GET");
            assert(request.getURI() == "/index.html");
            assert(request.getVersion() == "HTTP/1.1");
            assert(request.getHeader("Host") == "www.example.com");
            std::cout << "Valid request test passed." << std::endl;
        } catch (const HttpException& e) {
            std::cerr << "Valid request test failed: " << e.what() << std::endl;
        }
    }

    // Test invalid method
    {
        std::string rawRequest = "INVALID /index.html HTTP/1.1\r\nHost: www.example.com\r\n\r\n";
        HttpRequest request;
        try {
            request.parse(rawRequest);
            std::cerr << "Invalid method test failed: Exception not thrown" << std::endl;
        } catch (const HttpException& e) {
            std::cout << "Invalid method test passed: " << e.what() << std::endl;
        }
    }

    // Test missing URI
    {
        std::string rawRequest = "GET HTTP/1.1\r\nHost: www.example.com\r\n\r\n";
        HttpRequest request;
        try {
            request.parse(rawRequest);
            std::cerr << "Missing URI test failed: Exception not thrown" << std::endl;
        } catch (const MissingURIException& e) {
            std::cout << "Missing URI test passed: " << e.what() << std::endl;
        }
    }

    // Test invalid header format
    {
        std::string rawRequest = "GET /index.html HTTP/1.1\r\nInvalidHeader\r\n\r\n";
        HttpRequest request;
        try {
            request.parse(rawRequest);
            std::cerr << "Invalid header format test failed: Exception not thrown" << std::endl;
        } catch (const InvalidHeaderFormatException& e) {
            std::cout << "Invalid header format test passed: " << e.what() << std::endl;
        }
    }
}

void testHttpResponse() {
    std::cout << "\nTesting HttpResponse..." << std::endl;

    HttpResponse response;

    // Test setting status code and message
    response.setStatusCode(200);
    assert(response.getStatusCode() == 200);
    assert(response.getStatusMessage() == "OK");
    std::cout << "Status code and message test passed." << std::endl;

    // Test setting headers
    response.setHeader("Content-Type", "text/html");
    assert(response.getHeader("Content-Type") == "text/html");
    std::cout << "Header setting test passed." << std::endl;

    // Test setting body
	response.setBody("<html><body>Hello, World!</body></html>");
	assert(response.getBody() == "<html><body>Hello, World!</body></html>");
    std::cout << "Body setting test passed." << std::endl;

    // Test response string generation
    std::string expectedResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 38\r\n\r\n<html><body>Hello, World!</body></html>";
    assert(response.responseToString() == expectedResponse);
    std::cout << "Response string generation test passed." << std::endl;

    // Test 404 error generation
    std::string notFoundResponse = response.generate404Error("/nonexistent.html");
    assert(notFoundResponse.find("HTTP/1.1 404 Not Found") != std::string::npos);
    assert(notFoundResponse.find("/nonexistent.html") != std::string::npos);
    std::cout << "404 error generation test passed." << std::endl;

    // Test redirection generation
    std::string redirectResponse = response.generateRedirection("/new-location.html");
    assert(redirectResponse.find("HTTP/1.1 302 Found") != std::string::npos);
    assert(redirectResponse.find("Location: /new-location.html") != std::string::npos);
    std::cout << "Redirection generation test passed." << std::endl;
}

int main() {
    try {
        testHttpRequest();
        testHttpResponse();
        std::cout << "\nAll tests passed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}