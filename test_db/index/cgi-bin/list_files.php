<?php
header('Content-Type: text/html');

// Directory where files are located
$directory = '../uploads'; // Adjust the path to your uploads directory

if (!is_dir($directory)) {
    echo '<option disabled>Uploads directory not found</option>';
    exit;
}

$files = array_diff(scandir($directory), array('.', '..'));

if (empty($files)) {
    echo '<option disabled>No files available</option>';
    exit;
}

foreach ($files as $file) {
    // Ensure that only files are listed (exclude directories)
    if (is_file("$directory/$file")) {
        echo '<option value="' . htmlspecialchars($file) . '">' . htmlspecialchars($file) . '</option>';
    }
}
?>
