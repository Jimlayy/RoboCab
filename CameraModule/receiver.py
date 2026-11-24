import json, os, socket

PATH = "/tmp/vision_events.sock"
if os.path.exists(PATH):
    os.remove(PATH)
sock = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
sock.bind(PATH)
while True:
    msg = json.loads(sock.recv(1024))
    print(msg)   # e.g. {"event": "intersection", "side": "left"}