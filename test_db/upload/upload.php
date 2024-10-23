<?php
// Vérifie si le fichier a été soumis via POST
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    // Vérifie si le champ de fichier existe et qu'il n'y a pas d'erreur lors de l'upload
    if (isset($_FILES['file']) && $_FILES['file']['error'] == 0) {
        $uploadDir = './uploads/'; // Répertoire où enregistrer les fichiers
        $uploadFile = $uploadDir . basename($_FILES['file']['name']);

        // Vérifie que le répertoire de destination existe
        if (!is_dir($uploadDir)) {
            mkdir($uploadDir, 0777, true);
        }

        // Vérifie la taille du fichier (vous avez mentionné une limite de 5M)
        if ($_FILES['file']['size'] <= 5 * 1024 * 1024) { // 5MB
            // Vérifie si le fichier a été correctement déplacé
            if (move_uploaded_file($_FILES['file']['tmp_name'], $uploadFile)) {
                echo "Le fichier a été téléchargé avec succès.";
            } else {
                echo "Échec du téléchargement du fichier.";
            }
        } else {
            echo "Le fichier est trop volumineux.";
        }
    } else {
        echo "Aucun fichier n'a été téléchargé ou une erreur est survenue.";
    }
} else {
    echo "Seules les requêtes POST sont acceptées.";
}
?>