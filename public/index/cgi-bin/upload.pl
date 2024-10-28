#!/usr/bin/env perl
use strict;
use warnings;
use CGI;
use File::Basename;

my $cgi = CGI->new;
my $upload_dir = "./public/uploads";

# Set headers
print $cgi->header(-type => "text/html; charset=UTF-8");

# Get uploaded file
my $upload_file = $cgi->param("fileToUpload");
if ($upload_file) {
    my $filename = basename($upload_file);
    my $upload_path = "$upload_dir/$filename";
    
    # Save uploaded file to the server
    if (open my $out, '>', $upload_path) {
        binmode $out;
        while (my $chunk = <$upload_file>) {
            print $out $chunk;
        }
        close $out;

        # HTML response for success
        print qq{
            <!DOCTYPE html>
            <html lang="fr">
            <head>
                <meta charset="UTF-8">
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <title>Succès du Téléversement</title>
                <link rel="icon" href="./public/index/favicon.png" type="image/x-icon">
                <style>
                    body {
                        font-family: Arial, sans-serif;
                        background-color: #595959;
                        color: #333;
                        margin: 0;
                        padding: 0;
                    }
                    header {
                        background: #35424a;
                        color: #ffffff;
                        padding: 20px 0;
                        text-align: center;
                        position: relative;
                    }
                    header h1 {
                        margin: 0;
                        font-size: 2em;
                    }
                    .back-button {
                        position: absolute;
                        top: 20px;
                        left: 20px;
                        background: #5cb85c;
                        color: white;
                        padding: 10px 20px;
                        text-decoration: none;
                        border-radius: 5px;
                        font-size: 1em;
                        transition: background 0.3s;
                    }
                    .back-button:hover {
                        background: #4cae4c;
                    }
                    .container {
                        display: flex;
                        flex-direction: column;
                        align-items: center;
                        justify-content: center;
                        padding: 20px;
                    }
                    .content {
                        background: #ffffff;
                        border-radius: 8px;
                        box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
                        padding: 20px;
                        max-width: 600px;
                        text-align: center;
                    }
                </style>
            </head>
            <body>

            <header>
                <a href="/" class="back-button">Retour à l'index</a>
                <h1>📝 Téléversement Réussi</h1>
            </header>

            <div class="container">
                <div class="content">
                    <h2>Le fichier "$filename" a été uploadé avec succès !</h2>
                    <p>Merci d'avoir utilisé notre service de téléversement de fichiers.</p>
                    <a href="/" class="back-button">Retour à l'index</a>
                </div>
            </div>

            </body>
            </html>
        };
    } else {
        # Error response
        print_error_page("Erreur : Impossible de déplacer le fichier.");
    }
} else {
    # Error response for missing file
    print_error_page("Erreur : Aucun fichier sélectionné pour le téléversement.");
}

sub print_error_page {
    my ($error_message) = @_;
    print qq{
        <!DOCTYPE html>
        <html lang="fr">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <title>Erreur de Téléversement</title>
        </head>
        <body>
            <h1>$error_message</h1>
            <a href="/" class="back-button">Retour à l'index</a>
        </body>
        </html>
    };
}
