#!/usr/bin/env python3
import cgi
import cgitb
cgitb.enable()

print("Content-Type: text/html\n")
print("<html><body>")
print("<h1>CGI Test Script</h1>")

form = cgi.FieldStorage()
if "name" in form:
    print("<p>Hello, {}!</p>".format(form["name"].value))
else:
    print("<p>Hello, World!</p>")

print("</body></html>")