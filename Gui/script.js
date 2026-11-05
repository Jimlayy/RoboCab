let ws = null;   // vår WebSocket-anslutning

// Förifyll adressen: samma dator som sidan kom ifrån, annars localhost
document.getElementById("url").value =
  "ws://" + (location.hostname || "localhost") + ":8765";

function log(text) {
  const el = document.getElementById("log");
  el.textContent += text + "\n";
  el.scrollTop = el.scrollHeight;      // scrolla ner automatiskt
}

function setStatus(connected) {
  const s = document.getElementById("status");
  s.textContent = connected ? "● Ansluten" : "● Ej ansluten";
  s.className = connected ? "on" : "off";
}

function connect() {
  if (ws) ws.close();
  ws = new WebSocket(document.getElementById("url").value);

  // De fyra händelserna en WebSocket har:
  ws.onopen    = ()  => { setStatus(true);  log("[ansluten till bryggan]"); };
  ws.onmessage = (e) => log("Pi: " + e.data);       // svar från Pi:n via bryggan
  ws.onclose   = ()  => { setStatus(false); log("[frånkopplad]"); };
  ws.onerror   = ()  => log("[fel – körs bridge.py?]");
}

function send(text) {
  if (!ws || ws.readyState !== WebSocket.OPEN) { log("[inte ansluten]"); return; }
  if (!text) return;
  ws.send(text);
  log("Du: " + text);
}

// Enter i textrutan skickar
document.getElementById("msg").addEventListener("keydown", (e) => {
  if (e.key === "Enter") send(e.target.value);
});

connect();   // försök ansluta direkt när sidan laddas