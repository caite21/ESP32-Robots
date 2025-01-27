import http.server
import ssl


class OTARequestHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/ota.bin":
            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.end_headers()
            with open("ota.bin", "rb") as file:
                self.wfile.write(file.read())
        else:
            self.send_response(404)
            self.end_headers()
            self.wfile.write(b"File not found.")

# Server setup
server_address = ("0.0.0.0", 8070)
httpd = http.server.HTTPServer(server_address, OTARequestHandler)

ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
ssl_context.load_cert_chain(certfile="cert.pem", keyfile="key.pem")

httpd.socket = ssl_context.wrap_socket(httpd.socket, server_side=True)



print("Starting HTTPS server on port 8070. Serve `ota.bin` for OTA.")
try:
    httpd.serve_forever()
except KeyboardInterrupt:
    print("\nShutting down server.")
    httpd.shutdown()
