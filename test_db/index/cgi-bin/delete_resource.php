<?php
// Allow only DELETE requests
if ($_SERVER['REQUEST_METHOD'] !== 'DELETE') {
    http_response_code(405); // Method Not Allowed
    header('Content-Type: text/plain');
    echo '405 Method Not Allowed';
    exit;
}

// Parse the query string to get the 'resource' parameter
parse_str($_SERVER['QUERY_STRING'], $query);
if (!isset($query['resource']) || empty($query['resource'])) {
    http_response_code(400); // Bad Request
    header('Content-Type: text/plain');
    echo '400 Bad Request: No resource specified.';
    exit;
}

$resource = $query['resource'];
$resource = basename($resource); // Prevent directory traversal attacks

// Directory where files are located
$directory = '../uploads'; // Adjust the path to your uploads directory
$filePath = realpath($directory . '/' . $resource);

// Ensure that the file is within the uploads directory
$uploadsRealPath = realpath($directory);
if ($uploadsRealPath === false || strpos($filePath, $uploadsRealPath) !== 0) {
    http_response_code(403); // Forbidden
    header('Content-Type: text/plain');
    echo '403 Forbidden: Invalid file path.';
    exit;
}

// Check if the file exists and is a file
if (!is_file($filePath)) {
    http_response_code(404); // Not Found
    header('Content-Type: text/plain');
    echo '404 Not Found: File does not exist.';
    exit;
}

// Attempt to delete the file
if (unlink($filePath)) {
    http_response_code(200); // OK
    header('Content-Type: text/plain');
    echo 'File deleted successfully.';
} else {
    http_response_code(500); // Internal Server Error
    header('Content-Type: text/plain');
    echo '500 Internal Server Error: Could not delete the file.';
}
?>
