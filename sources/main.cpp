#include <iostream>
#include <string>
#include "../includes/HttpRequest.hpp"

void testRequest(const std::string& raw_request);

int main() {
    // Test des requêtes
    std::string raw_request_1 = "POST /api/v1/upload HTTP/1.1\r\n"
                                 "Host: www.example.com\r\n"
                                 "Content-Type: application/json\r\n"
                                 "Content-Length: 27\r\n"
                                 "Authorization: Bearer your_access_token_here\r\n"
                                 "User-Agent: Mozilla/5.0\r\n"
                                 "\r\n"
                                 "{\"name\": \"example\", \"type\": \"test\"}\r\n";

    std::string raw_request_2 = "POST /upload HTTP/1.1\r\n"
                                 "Host: upload.example.com\r\n"
                                 "Transfer-Encoding: chunked\r\n"
                                 "Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
                                 "User-Agent: Mozilla/5.0\r\n"
                                 "\r\n"
                                 "7\r\n"
                                 "file1=\r\n"
                                 "----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
                                 "Content-Disposition: form-data; name=\"file1\"; filename=\"example.txt\"\r\n"
                                 "Content-Type: text/plain\r\n"
                                 "\r\n"
                                 "This is the content of the file.\r\n"
                                 "----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
                                 "Content-Disposition: form-data; name=\"file2\"; filename=\"example2.txt\"\r\n"
                                 "Content-Type: text/plain\r\n"
                                 "\r\n"
                                 "Another file content.\r\n"
                                 "0\r\n"
                                 "\r\n";

    std::string raw_request_3 = "POST /api/v1/upload\r\n"
                                 "Host: www.example.com\r\n"
                                 "Content-Type: application/json\r\n"
                                 "Content-Length: 27\r\n"
                                 "Authorization: Bearer your_access_token_here\r\n"
                                 "User-Agent: Mozilla/5.0\r\n"
                                 "\r\n"
                                 "{\"name\": \"example\", \"type\": \"test\"}\r\n";

    std::string raw_request_4 = "POST /upload HTTP/1.1\r\n"
                                 "Host: upload.example.com\r\n"
                                 "Transfer-Encoding chunked\r\n"  // Erreur: manque les deux-points
                                 "Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
                                 "User-Agent: Mozilla/5.0\r\n"
                                 "\r\n"
                                 "7\r\n"
                                 "file1=\r\n"
                                 "----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
                                 "Content-Disposition: form-data; name=\"file1\"; filename=\"example.txt\"\r\n"
                                 "Content-Type: text/plain\r\n"
                                 "\r\n"
                                 "This is the content of the file.\r\n"
                                 "----WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
                                 "Content-Disposition: form-data; name=\"file2\"; filename=\"example2.txt\"\r\n"
                                 "Content-Type: text/plain\r\n"
                                 "\r\n"
                                 "Another file content.\r\n"
                                 "0\r\n"
                                 "\r\n";

    std::cout << "Testing raw_request_1:\n";
    testRequest(raw_request_1);

    std::cout << "\nTesting raw_request_2:\n";
    testRequest(raw_request_2);

    std::cout << "\nTesting raw_request_3:\n";
    testRequest(raw_request_3);

    std::cout << "\nTesting raw_request_4:\n";
    testRequest(raw_request_4);

    return 0;
}
