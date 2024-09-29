#include <iostream>
#include <string>
#include <cassert>
#include "../includes/HttpResponse.hpp"

void testChunkedEncodingWithTrailers() {
    std::cout << "Testing Chunked Encoding with Trailers..." << std::endl;
    std::string raw_response =
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/plain\r\n"
        "Trailer: X-Checksum\r\n"
        "\r\n"
        "b\r\n"
        "Hello World\r\n"
        "1a\r\n"
        ", welcome to chunked encoding!\r\n"
        "0\r\n"
        "X-Checksum: 43b5c9f1\r\n"
        "\r\n";

    HttpResponse response;
    try {
        response.parse(raw_response);
        std::cout << "Status Code: " << response.getStatusCode() << std::endl;
        std::cout << "Transfer-Encoding: " << response.getHeader("Transfer-Encoding") << std::endl;
        std::cout << "Parsed body: '" << response.getBody() << "'" << std::endl;
        std::cout << "Body length: " << response.getBody().length() << std::endl;
        std::cout << "X-Checksum: " << response.getHeader("X-Checksum") << std::endl;

        assert(response.getStatusCode() == 200);
        assert(response.getHeader("Transfer-Encoding") == "chunked");
        assert(response.getBody() == "Hello World, welcome to chunked encoding!");
        assert(response.getHeader("X-Checksum") == "43b5c9f1");
        std::cout << "Chunked Encoding with Trailers test passed." << std::endl;
    } catch (const HttpResponseException& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
    }
}

void testMultipartResponse() {
    std::cout << "Testing Multipart Response..." << std::endl;
    std::string raw_response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: multipart/mixed; boundary=frontier\r\n"
        "\r\n"
        "--frontier\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "This is the body of the message.\r\n"
        "--frontier\r\n"
        "Content-Type: application/octet-stream\r\n"
        "Content-Transfer-Encoding: base64\r\n"
        "\r\n"
        "PGh0bWw+CiAgPGhlYWQ+CiAgPC9oZWFkPgogIDxib2R5PgogICAgPHA+VGhpcyBpcyB0aGUg\r\n"
        "Ym9keSBvZiB0aGUgbWVzc2FnZS48L3A+CiAgPC9ib2R5Pgo8L2h0bWw+Cg==\r\n"
        "--frontier--\r\n";

    HttpResponse response;
    try {
        response.parse(raw_response);
        assert(response.getStatusCode() == 200);
        assert(response.getHeader("Content-Type").find("multipart/mixed") != std::string::npos);
        std::cout << "Multipart Response test passed." << std::endl;
    } catch (const HttpResponseException& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
    }
}

void testCompressedChunkedResponse() {
    std::cout << "Testing Compressed Chunked Response..." << std::endl;
    std::string raw_response =
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Encoding: gzip\r\n"
        "Content-Type: text/html\r\n"
        "\r\n"
        "8\r\n"
        "1f8b0800\r\n"
        "a\r\n"
        "0000000000\r\n"
        "c\r\n"
        "03c8e432c6ae\r\n"
        "d\r\n"
        "5438d15168a121\r\n"
        "7\r\n"
        "cad0510c\r\n"
        "0\r\n"
        "\r\n";

    HttpResponse response;
    try {
        response.parse(raw_response);
        assert(response.getStatusCode() == 200);
        assert(response.getHeader("Transfer-Encoding") == "chunked");
        assert(response.getHeader("Content-Encoding") == "gzip");
        std::cout << "Compressed Chunked Response test passed." << std::endl;
    } catch (const HttpResponseException& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
    }
}

void testMalformedHeaders() {
    std::cout << "Testing Malformed Headers..." << std::endl;
    std::string raw_response =
        "HTTP/1.1 200 OK\r\n"
        "content-type:text/plain\r\n"
        "Content-Length: 50\r\n"
        "X-Custom-Header: Value1, Value2\r\n"
        "X-Custom-Header: Value3\r\n"
        " Continuation-of-previous-header\r\n"
        "\r\n"
        "This is the body of the message. It is incomplete and";

    HttpResponse response;
    try {
        response.parse(raw_response);
        assert(response.getStatusCode() == 200);
        assert(response.getHeader("content-type") == "text/plain");
        assert(response.getHeader("X-Custom-Header") == "Value1, Value2, Value3 Continuation-of-previous-header");
        std::cout << "Malformed Headers test passed." << std::endl;
    } catch (const HttpResponseException& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
    }
}

void testNonStandardResponse() {
    std::cout << "Testing Non-Standard Response..." << std::endl;
    std::string raw_response =
        "HTTP/2.0 299 Custom Status\r\n"
        "Date: Mon, 23 May 2023 22:38:34 GMT\r\n"
        "Server: CustomServer/1.0\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 57\r\n"
        "\r\n"
        "{\"status\": \"semi-ok\", \"message\": \"This is a non-standard response\"}";

    HttpResponse response;
    try {
        response.parse(raw_response);
        assert(response.getHTTPVersion() == "HTTP/2.0");
        assert(response.getStatusCode() == 299);
        assert(response.getStatusMessage() == "Custom Status");
        assert(response.getHeader("Content-Type") == "application/json");
        std::cout << "Non-Standard Response test passed." << std::endl;
    } catch (const HttpResponseException& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
    }
}

int main() {
    // testChunkedEncodingWithTrailers();
    testMultipartResponse();
    testCompressedChunkedResponse();
    testMalformedHeaders();
    testNonStandardResponse();

    return 0;
}