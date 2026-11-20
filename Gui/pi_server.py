"""
pi_server.py  –  runs on the Raspberry Pi

A simple TCP server. It waits for the bridge to connect, reads commands
(one line of text per command, terminated by '\n') and sends back a reply
(also one line of text).

Start:  python3 pi_server.py
"""

import socket
import time

HOST = "0.0.0.0"   # listen on all network interfaces (so other computers can reach us)
PORT = 5000        # the port the bridge connects to

led_on = False     # fake state; replace with real GPIO code if you want

C_SOCK_PATH = "/tmp/robocab_cmd.sock"   # must match SOCK_PATH in cmd_reader.c

# Datagram socket: each sendto() becomes exactly one message for the C thread,
# so we need no line breaks or buffering on the C side.
c_sock = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)


def send_to_c(cmd: str) -> bool:
    """Sends a command to the C thread. Returns False if it is not listening."""
    try:
        c_sock.sendto(cmd.encode("utf-8"), C_SOCK_PATH)
        return True
    except (FileNotFoundError, ConnectionRefusedError):
        print("Varning: C-programmet lyssnar inte på", C_SOCK_PATH)
        return False


def handle_command(cmd: str) -> str:
    """Takes a command as text and returns the reply as text."""
    cmd = cmd.strip().upper()

    match cmd.split():
        case ["TIME"]:
            return "TIME " + time.strftime("%H:%M:%S")
        case ["FORWARD"]:
            send_to_c(cmd)
            return "Skickar kommandot framåt"
        case ["BACK"]:
            send_to_c(cmd)
            return "Skickar kommandot back"
        case ["LEFT"]:
            send_to_c(cmd)
            return "Skickar kommandot vänster"
        case ["RIGHT"]:
            send_to_c(cmd)
            return "Skickar kommandot höger"
        case ["STOP"]:
            send_to_c(cmd)
            return "Skickar kommandot bromsa!"
        case ["AUTO", "ON"]:
            send_to_c(cmd)
            return "Byter till autonomt körläge"
        case ["AUTO", "OFF"]:
            send_to_c(cmd)
            return "Byter till manuellt körläge"
        case ["START", start, goal]:
            send_to_c(cmd)
            return f"SKickar information om upphämtning {start} och avlämning {goal}"
        case _:
            return "OKÄNT KOMMANDO: " + cmd


def main():
    # 1. Create a TCP socket (AF_INET = IPv4, SOCK_STREAM = TCP)
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # Lets us restart the server right away without "Address already in use"
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    # 2. Bind it to an address + port and start listening
    server.bind((HOST, PORT))
    server.listen()
    print(f"Pi-servern lyssnar på port {PORT} ...")

    while True:
        # 3. Wait until someone (the bridge) connects
        conn, addr = server.accept() # When a new connection is established, the connection and adress is saved.
        print("Ansluten:", addr)

        # makefile lets us read line by line, like from a regular file
        with conn, conn.makefile("r", encoding="utf-8") as reader:
            for line in reader:              # the loop ends when the bridge disconnects
                print("Fick:", line.strip())
                # put line.strip() on socket
                answer = handle_command(line)
                conn.sendall((answer + "\n").encode("utf-8"))

        print("Frånkopplad:", addr)


if __name__ == "__main__":
    main()