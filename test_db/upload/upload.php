<?php
// Vérifier si le formulaire a été soumis
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    // Dossier où les fichiers seront enregistrés
    $target_dir = "uploads/";

    // Créer le dossier s'il n'existe pas
    if (!file_exists($target_dir)) {
        mkdir($target_dir, 0755, true);
    }
    
    // Variable pour vérifier si l'upload est valide
    $uploadOk = 1;
    
    // Types de fichiers autorisés (extensions)
    $allowedFileTypes = array('jpg', 'jpeg', 'png', 'gif', 'pdf', 'doc', 'docx');
    
    // Obtenir l'extension du fichier de manière sécurisée
    $fileExtension = strtolower(pathinfo($_FILES["fileToUpload"]["name"], PATHINFO_EXTENSION));

    // Générer un nom de fichier unique avec l'extension appropriée
    $newFileName = uniqid() . '.' . $fileExtension;
    $target_file = $target_dir . $newFileName;

    // Vérifier si le fichier existe déjà
    if (file_exists($target_file)) {
        echo "Désolé, le fichier existe déjà.";
        $uploadOk = 0;
    }
    
    // Vérifier la taille du fichier (par exemple, max 5 Mo)
    if ($_FILES["fileToUpload"]["size"] > 5000000) {
        echo "Désolé, votre fichier est trop volumineux.";
        $uploadOk = 0;
    }
    
    // Vérifier si l'extension du fichier est autorisée
    if (!in_array($fileExtension, $allowedFileTypes)) {
        echo "Désolé, seuls les fichiers de type " . implode(', ', $allowedFileTypes) . " sont autorisés.";
        $uploadOk = 0;
    }
    
    // Obtenir le type MIME réel du fichier
    $mimeType = mime_content_type($_FILES["fileToUpload"]["tmp_name"]);

    // Liste des types MIME autorisés
    $allowedMimeTypes = array(
        'image/jpeg',
        'image/png',
        'image/gif',
        'application/pdf',
        'application/msword',
        'application/vnd.openxmlformats-officedocument.wordprocessingml.document'
    );

    // Vérifier si le type MIME est autorisé
    if (!in_array($mimeType, $allowedMimeTypes)) {
        echo "Désolé, seuls les fichiers de type " . implode(', ', $allowedFileTypes) . " sont autorisés.";
        $uploadOk = 0;
    }

    // Vérifier si $uploadOk est à 0 suite à une erreur
    if ($uploadOk == 0) {
        echo "Désolé, votre fichier n'a pas été uploadé.";
    } else {
        // Si tout est OK, tenter d'uploader le fichier
        if (move_uploaded_file($_FILES["fileToUpload"]["tmp_name"], $target_file)) {
            echo "Le fichier ". htmlspecialchars(basename($_FILES["fileToUpload"]["name"])). " a été uploadé avec succès.";
        } else {
            echo "Désolé, une erreur est survenue lors de l'upload de votre fichier.";
        }
    }
}
?>

<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <title>Uploader un fichier</title>
</head>
<body>
    <h2>Uploader un fichier</h2>
    <form action="upload.php" method="post" enctype="multipart/form-data">
        Sélectionnez le fichier à uploader :
        <br><br>
        <input type="file" name="fileToUpload" id="fileToUpload">
        <br><br>
        <input type="submit" value="Uploader le fichier" name="submit">
    </form>
</body>
</html>
