#!/usr/bin/env python3

import os
import urllib.parse

# Required HTTP headers
print("Content-Type: text/plain; charset=utf-8\r\n\r\n")

# Path to the uploads folder
upload_dir = './www/main/cgi-bin/uploads/'

# Check if the request method is DELETE
if os.environ.get('REQUEST_METHOD') == 'DELETE':
    # Extract the resource from the query string
    query_string = os.environ.get('QUERY_STRING', '')
    params = urllib.parse.parse_qs(query_string)
    resource = params.get('resource', [''])[0]  # Get the 'resource' parameter

    # Construct the file path
    file_path = os.path.join(upload_dir, resource)

    # Check if the file exists and delete it
    if resource and os.path.isfile(file_path):
        try:
            os.remove(file_path)  # Delete the file
            print(f"Resource '{resource}' has been deleted successfully.")
        except Exception as e:
            print(f"Error: Could not delete resource '{resource}'. {e}")
    else:
        print(f"Error: Resource '{resource}' does not exist.")
else:
    print("Error: Invalid request method. Use DELETE.")