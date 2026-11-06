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

 let start = null;
let stop = null;

function updateNodeColors(start, stop) {
  document.querySelectorAll(".node").forEach(node => {
    const id = node.dataset.id;
    node.classList.toggle("start", id === start);
    node.classList.toggle("stop", id === stop);
  });
}
document.querySelectorAll(".node").forEach(node => {
  node.addEventListener("click", () => {
    const id = node.dataset.id;

    if (start === null || stop !== null) {
      start = id;
      stop = null;
    } else if (id !== start) {
      stop = id;
    }

    updateNodeColors(start, stop);
    updateStartButton();
  });
});


const startBtn = document.getElementById("start-btn");
const statusEl = document.getElementById("status");

// Start can only be pressed when connected and both a start and a stop are chosen
function updateStartButton() {
  const connected = statusEl.classList.contains("on");
  startBtn.disabled = !(connected && start !== null && stop !== null);
}

startBtn.addEventListener("click", () => {
  send(`START ${start} ${stop}`);
});

// Re-check whenever the connection status changes (script.js switches its class on/off)
new MutationObserver(updateStartButton).observe(statusEl, { attributes: true, attributeFilter: ["class"] });

updateStartButton();
