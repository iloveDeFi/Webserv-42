#!/usr/bin/env python3
import os

# Directory where the files are stored
upload_dir = './www/main/cgi-bin/uploads/'

# Required HTTP headers
print("Content-Type: text/html\n")

# Generate the HTML for the select options
try:
    files = os.listdir(upload_dir)
    
    if files:
        for file in files:
            filepath = os.path.join(upload_dir, file)
            if os.path.isfile(filepath):
                print(f'<option value="{file}">{file}</option>')
    else:
        print('<option disabled>No files available</option>')

except Exception as e:
    print(f'<option disabled>Error loading files: {str(e)}</option>')