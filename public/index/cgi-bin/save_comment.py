#!/usr/bin/env python3

import os
import cgi
import json
import sys
from datetime import datetime

UPLOAD_DIR = "./public/uploads"

# Ensure the upload directory exists
if not os.path.exists(UPLOAD_DIR):
    os.makedirs(UPLOAD_DIR)

# Get the HTTP request method
request_method = os.environ.get("REQUEST_METHOD", "GET")

# Output JSON header once
print("Content-Type: application/json\r\n")  # Ajoute \r\n explicitement après chaque ligne d'en-tête
print("\r\n", end="")  # Sépare les en-têtes du corps avec \r\n\r\n

# Handle POST and GET requests
response = {}  # Declare an empty response dictionary

if request_method == "POST":
    # Check content type to determine how to read the data
    content_type = os.environ.get("CONTENT_TYPE", "")
    if "application/json" in content_type:
        length = int(os.environ.get("CONTENT_LENGTH", 0))
        raw_data = sys.stdin.read(length)  # Lire les données brutes
        try:
            # Parse the JSON data
            data = json.loads(raw_data)
            comment = data.get("comment", "").strip()
        except json.JSONDecodeError:
            response = {
                "status": "error",
                "message": "Invalid JSON format."
            }
            print(json.dumps(response))
            sys.exit(0)  # Exit early if JSON is invalid
    else:
        # Fallback for form data
        form = cgi.FieldStorage()
        comment = form.getvalue("comment", "").strip()

    if comment:
        timestamp = datetime.now().strftime("%Y%m%d%H%M%S")
        filename = f"comment_{timestamp}.txt"
        filepath = os.path.join(UPLOAD_DIR, filename)

        # Security measure to prevent directory traversal attacks
        if os.path.abspath(filepath).startswith(os.path.abspath(UPLOAD_DIR)):
            with open(filepath, "w") as file:
                file.write(comment)

            response = {
                "status": "success",
                "message": "Comment saved successfully."
            }
        else:
            response = {
                "status": "error",
                "message": "Invalid file path."
            }
    else:
        response = {
            "status": "error",
            "message": "No comment provided."
        }

elif request_method == "GET":
    comments = []
    try:
        for filename in os.listdir(UPLOAD_DIR):
            if filename.endswith(".txt"):
                filepath = os.path.join(UPLOAD_DIR, filename)
                with open(filepath, "r") as file:
                    comments.append(file.read().strip())

        response = {
            "status": "success",
            "comments": comments
        }
    except Exception as e:
        response = {
            "status": "error",
            "message": f"Failed to load comments: {str(e)}"
        }

print(json.dumps(response))  # Print only JSON here
