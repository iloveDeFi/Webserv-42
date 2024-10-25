<?php
$uploadDir = './public/uploads/'; 

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    if (isset($_FILES['fileToUpload']) && $_FILES['fileToUpload']['error'] === UPLOAD_ERR_OK) {
        $uploadedFile = $_FILES['fileToUpload'];
        $fileName = basename($uploadedFile['name']);
        $destination = $uploadDir . $fileName;

        if (move_uploaded_file($uploadedFile['tmp_name'], $destination)) {
            echo "Content-Type: text/html\r\n\r\n";
            echo "<html><body>Le fichier " . htmlspecialchars($fileName) . " a été uploadé avec succès !</body></html>";
        } else {
            echo "Content-Type: text/html\r\n\r\n";
            echo "<html><body>Erreur : Impossible de déplacer le fichier.</body></html>";
        }
    } else {
        echo "Content-Type: text/html\r\n\r\n";
        echo "<html><body>Erreur : Aucun fichier n'a été uploadé ou une erreur est survenue.</body></html>";
    }
} else {
    echo "Content-Type: text/html\r\n\r\n";
    echo "<html><body>Erreur : La méthode de requête n'est pas supportée.</body></html>";
}
?>
