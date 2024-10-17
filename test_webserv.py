import unittest
import requests
import subprocess

class WebServerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # Lancer le serveur avant d'exécuter les tests
        cls.server_process = subprocess.Popen(['./webserv', 'config.yaml'], stdout=subprocess.PIPE)
    
    @classmethod
    def tearDownClass(cls):
        # Arrêter le serveur après les tests
        cls.server_process.terminate()

    # Test GET Request
    def test_get_request(self):
        response = requests.get('http://localhost:8080')
        self.assertEqual(response.status_code, 200)
        self.assertIn("Expected content", response.text)
    
    # Test POST Request
    def test_post_request(self):
        data = {'key': 'value'}
        response = requests.post('http://localhost:8080/upload', data=data)
        self.assertEqual(response.status_code, 200)
        self.assertIn("File uploaded successfully", response.text)
    
    # Test DELETE Request
    def test_delete_request(self):
        response = requests.delete('http://localhost:8080/delete/file.txt')
        self.assertIn(response.status_code, [200, 204])
        self.assertIn("File deleted successfully", response.text)
    
    # Test Static File Serving
    def test_static_file_serving(self):
        response = requests.get('http://localhost:8080/static/index.html')
        self.assertEqual(response.status_code, 200)
        self.assertIn("<html>", response.text)
    
    # Test Directory Listing
    def test_directory_listing(self):
        response = requests.get('http://localhost:8080/static/')
        self.assertEqual(response.status_code, 200)
        self.assertIn("Index of /static", response.text)
    
    # Test 404 Response
    def test_404_response(self):
        response = requests.get('http://localhost:8080/unknown_page')
        self.assertEqual(response.status_code, 404)
        self.assertIn("404 Not Found", response.text)

    # Test Redirect (301)
    def test_redirect(self):
        response = requests.get('http://localhost:8080/old_page', allow_redirects=False)
        self.assertEqual(response.status_code, 301)
        self.assertEqual(response.headers['Location'], 'http://localhost:8080/new_page')

    # Test File Upload
    def test_file_upload(self):
        files = {'file': open('test.txt', 'rb')}
        response = requests.post('http://localhost:8080/upload', files=files)
        self.assertEqual(response.status_code, 200)
        self.assertIn("File uploaded successfully", response.text)

    # Test Security (Access to protected files)
    def test_protected_file_access(self):
        response = requests.get('http://localhost:8080/../etc/passwd')
        self.assertEqual(response.status_code, 403)

# Fonction principale pour exécuter les tests
if __name__ == '__main__':
    unittest.main()
