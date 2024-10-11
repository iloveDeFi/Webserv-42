    HttpRequest simulatedRequest;

    simulatedRequest.setMethod("GET");
    simulatedRequest.setURI("/index.html");
    simulatedRequest.setHTTPVersion("HTTP/1.1");

    std::map<std::string, std::string> headers;
    headers["Host"] = "localhost";
    headers["Connection"] = "keep-alive";
    simulatedRequest.setHeaders(headers);
    simulatedRequest.setBody("");
    simulatedRequest.setIsChunked(false);

    std::cout << "Method: " << simulatedRequest.getMethod() << std::endl;
    std::cout << "URI: " << simulatedRequest.getURI() << std::endl;
    std::cout << "HTTP Version: " << simulatedRequest.getHTTPVersion() << std::endl;

    return 0;
}

