"""
pi_server.py  –  körs på Raspberry Pi:n

En enkel TCP-server. Den väntar på att bryggan ansluter, läser kommandon
(en rad text per kommando, avslutad med '\n') och skickar tillbaka ett svar
(också en rad text).

Starta:  python3 pi_server.py
"""

import socket
import time

HOST = "0.0.0.0"   # lyssna på alla nätverkskort (så att andra datorer når oss)
PORT = 5000        # porten bryggan ska ansluta till

led_on = False     # låtsas-tillstånd; byt mot riktig GPIO-kod om du vill


def handle_command(cmd: str) -> str:
    """Tar emot ett kommando som text och returnerar svaret som text."""
    global led_on
    cmd = cmd.strip().upper()

    if cmd == "PING":
        return "PONG"
    if cmd == "TIME":
        return "TIME " + time.strftime("%H:%M:%S")
    if cmd == "LED ON":
        led_on = True
        # Här skulle du t.ex. göra: GPIO.output(17, GPIO.HIGH)
        return "LED är PÅ"
    if cmd == "LED OFF":
        led_on = False
        # Här skulle du t.ex. göra: GPIO.output(17, GPIO.LOW)
        return "LED är AV"
    if cmd == "STATUS":
        return "LED " + ("PÅ" if led_on else "AV")
    return "OKÄNT KOMMANDO: " + cmd


def main():
    # 1. Skapa en TCP-socket (AF_INET = IPv4, SOCK_STREAM = TCP)
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # Låter oss starta om servern direkt utan "Address already in use"
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    # 2. Knyt den till en adress + port och börja lyssna
    server.bind((HOST, PORT))
    server.listen()
    print(f"Pi-servern lyssnar på port {PORT} ...")

    while True:
        # 3. Vänta tills någon (bryggan) ansluter
        conn, addr = server.accept()
        print("Ansluten:", addr)

        # makefile gör att vi kan läsa rad för rad, som från en vanlig fil
        with conn, conn.makefile("r", encoding="utf-8") as reader:
            for line in reader:              # loopen slutar när bryggan kopplar ner
                print("Fick:", line.strip())
                answer = handle_command(line)
                conn.sendall((answer + "\n").encode("utf-8"))

        print("Frånkopplad:", addr)


if __name__ == "__main__":
    main()
