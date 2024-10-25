#!/usr/bin/env python3

import os
import cgi
import cgitb
import json
from datetime import datetime

cgitb.enable()  # Enable debugging

UPLOAD_DIR = "./public/uploads"  # Path to the upload directory

# Ensure the upload directory exists
if not os.path.exists(UPLOAD_DIR):
    os.makedirs(UPLOAD_DIR)

# Get the HTTP request method
request_method = os.environ.get("REQUEST_METHOD", "GET")

# Set the content-type for JSON response
print("Content-Type: application/json")
print()  # Blank line separating headers from body

# Handle POST request (Save new comment)
if request_method == "POST":
    form = cgi.FieldStorage()
    comment = form.getvalue("comment", "").strip()
    
    if comment:
        # Create a new .txt file for the comment
        timestamp = datetime.now().strftime("%Y%m%d%H%M%S")
        filename = f"comment_{timestamp}.txt"
        filepath = os.path.join(UPLOAD_DIR, filename)
        
        with open(filepath, "w") as file:
            file.write(comment)
        
        # Respond with success message in JSON
        response = {
            "status": "success",
            "message": "Comment saved successfully."
        }
        print(json.dumps(response))
    else:
        # Respond with error if no comment text
        response = {
            "status": "error",
            "message": "No comment provided."
        }
        print(json.dumps(response))

# Handle GET request (Load all comments)
else:
    comments = []
    
    # Read all .txt files from the upload directory
    for filename in os.listdir(UPLOAD_DIR):
        if filename.endswith(".txt"):
            filepath = os.path.join(UPLOAD_DIR, filename)
            with open(filepath, "r") as file:
                comments.append(file.read().strip())
    
    # Respond with all comments in JSON format
    response = {
        "status": "success",
        "comments": comments
    }
    print(json.dumps(response))
