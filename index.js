const { WebSocketServer } = require("ws");
const wss = new WebSocketServer({ port: 3000 });

function morseToText(morseArr) {
  const morseTable = {
    ".-": "A",
    "-...": "B",
    "-.-.": "C",
    "-..": "D",
    ".": "E",
    "..-.": "F",
    "--.": "G",
    "....": "H",
    "..": "I",
    ".---": "J",
    "-.-": "K",
    ".-..": "L",
    "--": "M",
    "-.": "N",
    "---": "O",
    ".--.": "P",
    "--.-": "Q",
    ".-.": "R",
    "...": "S",
    "-": "T",
    "..-": "U",
    "...-": "V",
    ".--": "W",
    "-..-": "X",
    "-.--": "Y",
    "--..": "Z",
  };
  if (!Array.isArray(morseArr)) return "";
  const decoded = morseArr.map((code) => morseTable[code] || "?").join("");
  console.log("Decoded message:", decoded);
  return decoded;
}

const clients = [];

wss.on("connection", (ws) => {
  if (clients.length < 2) {
    clients.push(ws);
    console.log("Client connected. Total:", clients.length);
  } else {
    ws.close(1000, "Only two clients allowed.");
    return;
  }

  ws.on("close", () => {
    const idx = clients.indexOf(ws);
    if (idx !== -1) clients.splice(idx, 1);
    console.log("Client disconnected. Total:", clients.length);
  });

  ws.on("message", (msg) => {
    let m;
    try {
      m = JSON.parse(msg.toString());
    } catch (e) {
      console.log("Invalid message format, expected JSON array.");
      return;
    }
    console.log("Received Morse:", m);
    const text = morseToText(m);
    // Send to the other client only
    clients.forEach((client) => {
      if (client !== ws && client.readyState === WebSocket.OPEN) {
        client.send(text);
      }
    });
  });
});

// Exporting the morseToText function for use in other files
module.exports = { morseToText };
