"""
bridge.py  –  körs på din dator (eller på Pi:n själv)

Bryggan är en "översättare":
  webbläsare  <--WebSocket-->  bridge.py  <--TCP-->  Raspberry Pi

Webbläsare kan INTE öppna vanliga TCP-socketar, bara WebSockets.
Därför behövs något i mitten som tar emot WebSocket-meddelanden och
skickar dem vidare som TCP (och tvärtom).

Installera:  pip install websockets
Starta:      python3 bridge.py
"""

import asyncio
import websockets

WS_HOST = "0.0.0.0"     # där webbsidan ansluter
WS_PORT = 8765

PI_HOST = "10.42.0.1"   # <-- BYT till Pi:ns IP, t.ex. "192.168.1.42"
PI_PORT = 5000


async def handle_browser(websocket):
    """Körs en gång för varje webbläsare som ansluter."""
    print("Webbläsare ansluten")

    # 1. Öppna en TCP-anslutning till Pi:n
    try:
        reader, writer = await asyncio.open_connection(PI_HOST, PI_PORT)
    except OSError as e:
        await websocket.send(f"FEL: kan inte nå Pi:n på {PI_HOST}:{PI_PORT} ({e})")
        return
    await websocket.send("Bryggan är ansluten till Pi:n ✔")

    # 2a. Webbläsare -> Pi
    async def browser_to_pi():
        async for message in websocket:      # väntar på varje nytt meddelande
            print("Webb -> Pi:", message)
            writer.write((message + "\n").encode("utf-8"))
            await writer.drain()             # se till att det faktiskt skickas

    # 2b. Pi -> webbläsare
    async def pi_to_browser():
        while True:
            line = await reader.readline()   # väntar på en hel rad från Pi:n
            if not line:                     # tom = Pi:n stängde anslutningen
                await websocket.send("Pi:n kopplade ner")
                break
            text = line.decode("utf-8").strip()
            print("Pi -> Webb:", text)
            await websocket.send(text)

    # 3. Kör båda riktningarna samtidigt. När den ena tar slut stoppar vi den andra.
    tasks = [asyncio.create_task(browser_to_pi()),
             asyncio.create_task(pi_to_browser())]
    try:
        await asyncio.wait(tasks, return_when=asyncio.FIRST_COMPLETED)
    except websockets.ConnectionClosed:
        pass
    finally:
        for t in tasks:
            t.cancel()
        writer.close()
        print("Webbläsare frånkopplad")


async def main():
    async with websockets.serve(handle_browser, WS_HOST, WS_PORT):
        print(f"Bryggan lyssnar på ws://localhost:{WS_PORT}  ->  Pi {PI_HOST}:{PI_PORT}")
        await asyncio.Future()   # kör för evigt


if __name__ == "__main__":
    asyncio.run(main())
