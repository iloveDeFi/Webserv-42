#!/usr/bin/env python

import cgi
import cgitb
import os

# Activer le débogage
cgitb.enable()

# Créer un FieldStorage pour récupérer les données du formulaire
form = cgi.FieldStorage()

# Vérifiez le contenu de la requête
print("Content-Type: text/plain")  # Changez ceci à text/plain pour voir le texte brut
print()  # Ligne vide requise pour le CGI
print("Débogage des données du formulaire:")
for key in form.keys():
    print(f"{key}: {form[key].value if form[key].file is None else 'Fichier présent'}")

# Vérifiez si un fichier a été téléchargé
if "file" in form:
    fileitem = form["file"]
    
    if fileitem.file:
        # Créez le chemin de téléchargement
        upload_path = os.path.join(os.path.abspath("public/uploads"), fileitem.filename)  # Utiliser le chemin absolu
        
        # Vérifiez si le fichier existe déjà
        if os.path.exists(upload_path):
            print("Status: 409 Conflict")
            print("Content-Type: text/html")
            print()
            print("<html><body><h1>Erreur : Le fichier existe déjà.</h1></body></html>")
        else:
            # Créez le répertoire s'il n'existe pas déjà
            os.makedirs(os.path.dirname(upload_path), exist_ok=True)

            # Enregistrez le fichier téléchargé
            with open(upload_path, 'wb') as f:
                f.write(fileitem.file.read())
            
            # Réponse de succès
            print("Content-Type: text/html")
            print()
            print("<html><body><h1>Upload réussi!</h1></body></html>")
    else:
        print("Content-Type: text/html")
        print()
        print("<html><body><h1>Aucun fichier n'a été téléchargé.</h1></body></html>")
else:
    print("Content-Type: text/html")
    print()
    print("<html><body><h1>Erreur : Aucune donnée reçue.</h1></body></html>")