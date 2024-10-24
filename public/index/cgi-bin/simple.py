#!/usr/bin/env python3
import os
import cgi
import html

# Récupérer les arguments GET
form = cgi.FieldStorage()
name = form.getvalue("name")

# Afficher les en-têtes HTTP
print("Content-Type: text/html")
print()

# Générer du contenu HTML en fonction de la requête
print("<html>")
print("<head>")
print("<title>Réponse CGI</title>")
print("</head>")
print("<body>")
print("<h1>Réponse du script CGI</h1>")

if name:
    # Si un nom est passé dans la requête GET
    escaped_name = html.escape(name)
    print(f"<p>Bonjour, {escaped_name} ! Merci d'utiliser notre serveur CGI.</p>")
else:
    # Si aucun nom n'est passé
    print("<p>Bonjour ! Vous n'avez pas fourni de nom.</p>")

print("</body>")
print("</html>")
