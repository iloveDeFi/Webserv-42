#!/usr/bin/env python3

import os
import cgi
import json
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
    form = cgi.FieldStorage()
    comment = form.getvalue("comment", "").strip()

    if comment:
        timestamp = datetime.now().strftime("%Y%m%d%H%M%S")
        filename = f"comment_{timestamp}.txt"
        filepath = os.path.join(UPLOAD_DIR, filename)

        with open(filepath, "w") as file:
            file.write(comment)

        response = {
            "status": "success",
            "message": "Comment saved successfully."
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
