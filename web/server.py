#!/usr/bin/env python3
"""
Simple HTTP Server for SDRS Web UI
Serves static files and proxies API requests to backend
"""

import http.server
import socketserver
import os
import sys
import urllib.request
import urllib.error
import json

PORT = 3000
DIRECTORY = os.path.dirname(os.path.abspath(__file__))
API_GATEWAY_URL = 'http://localhost:8080'

class MyHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)
    
    def do_GET(self):
        # Proxy API requests to backend
        if self.path.startswith('/health') or self.path.startswith('/api/'):
            self.proxy_request('GET')
        else:
            super().do_GET()
    
    def do_POST(self):
        # Proxy API requests to backend
        if self.path.startswith('/api/'):
            self.proxy_request('POST')
        else:
            self.send_error(405, "Method Not Allowed")
    
    def do_PUT(self):
        # Proxy API requests to backend
        if self.path.startswith('/api/'):
            self.proxy_request('PUT')
        else:
            self.send_error(405, "Method Not Allowed")
    
    def do_DELETE(self):
        # Proxy API requests to backend
        if self.path.startswith('/api/'):
            self.proxy_request('DELETE')
        else:
            self.send_error(405, "Method Not Allowed")
    
    def do_OPTIONS(self):
        # Handle preflight requests
        self.send_response(204)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type, Authorization')
        self.end_headers()
    
    def proxy_request(self, method):
        """Forward request to API Gateway"""
        try:
            # Build target URL
            target_url = API_GATEWAY_URL + self.path
            
            # Prepare request
            headers = {
                'Content-Type': 'application/json',
                'User-Agent': 'SDRS-Web-Proxy/1.0'
            }
            
            # Read request body for POST/PUT
            data = None
            if method in ['POST', 'PUT']:
                content_length = int(self.headers.get('Content-Length', 0))
                if content_length > 0:
                    data = self.rfile.read(content_length)
            
            # Make request to API Gateway
            req = urllib.request.Request(
                target_url,
                data=data,
                headers=headers,
                method=method
            )
            
            # Send request and get response
            with urllib.request.urlopen(req, timeout=10) as response:
                response_data = response.read()
                
                # Send response back to browser
                self.send_response(response.status)
                self.send_header('Content-Type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(response_data)
                
        except urllib.error.HTTPError as e:
            # Forward error response
            error_data = e.read()
            self.send_response(e.code)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(error_data)
            
        except urllib.error.URLError as e:
            # Connection error to API Gateway
            error_response = {
                'success': False,
                'message': f'Cannot connect to API Gateway: {str(e.reason)}',
                'status_code': 503
            }
            self.send_response(503)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(error_response).encode())
            
        except Exception as e:
            # General error
            error_response = {
                'success': False,
                'message': f'Proxy error: {str(e)}',
                'status_code': 500
            }
            self.send_response(500)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(error_response).encode())
    
    def end_headers(self):
        # Add CORS headers for static files only
        if not (self.path.startswith('/health') or self.path.startswith('/api/')):
            self.send_header('Access-Control-Allow-Origin', '*')
            self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
            self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        super().end_headers()

def main():
    print(f"Starting SDRS Web UI Server...")
    print(f"Directory: {DIRECTORY}")
    print(f"Port: {PORT}")
    print(f"\nAccess the application at:")
    print(f"  http://localhost:{PORT}/")
    print(f"  http://localhost:{PORT}/index.html")
    print(f"\nPress Ctrl+C to stop the server\n")
    
    with socketserver.TCPServer(("", PORT), MyHTTPRequestHandler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n\nShutting down server...")
            httpd.shutdown()
            sys.exit(0)

if __name__ == "__main__":
    main()
