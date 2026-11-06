
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
STREAM_PORT = 8000

class FrameBuffer:
    """Håller senaste JPEG-bilden. Varje tittare får alltid den nyaste, ingen kö byggs upp."""
    def __init__(self):
        self.jpeg = None
        self.cond = threading.Condition()

    def put(self, jpeg: bytes):
        with self.cond:
            self.jpeg = jpeg
            self.cond.notify_all()

    def get(self) -> bytes:
        with self.cond:
            self.cond.wait(timeout=1.0)
            return self.jpeg


frames = FrameBuffer()


class StreamHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path != "/stream.mjpg":
            self.send_error(404)
            return
        self.send_response(200)
        self.send_header("Cache-Control", "no-cache")
        self.send_header("Content-Type", "multipart/x-mixed-replace; boundary=frame")
        self.end_headers()
        try:
            while True:
                jpeg = frames.get()
                if jpeg is None:
                    continue
                self.wfile.write(b"--frame\r\nContent-Type: image/jpeg\r\n")
                self.wfile.write(f"Content-Length: {len(jpeg)}\r\n\r\n".encode())
                self.wfile.write(jpeg)
                self.wfile.write(b"\r\n")
        except (BrokenPipeError, ConnectionResetError):
            pass  # webbläsaren stängde fliken

    def log_message(self, *args):
        pass  # tyst, annars loggas varje anslutning

def stream():
    server = ThreadingHTTPServer(("0.0.0.0", STREAM_PORT), StreamHandler)
    threading.Thread(target=server.serve_forever, daemon=True).start()
    print(f"Videoström på http://<pi-ip>:{STREAM_PORT}/stream.mjpg")
