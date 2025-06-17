const { WebSocketServer } = require('ws');
const wss = new WebSocketServer({ port: 3000 });

function morseToText(morse) { 
  return "morse traduction placeholder"; // Placeholder for morse to text conversion logic
}

wss.on('connection', (ws) => {
  ws.on('message', (msg) => {
    const m = msg.toString();
    console.log('Received Morse:', m);
    const text = morseToText(m);
    // Broadcast to all other clients, or to specific one
    wss.clients.forEach(client => {
      if (client !== ws && client.readyState === WebSocket.OPEN) {
        client.send(text);
      }
    });
  });
});

// Exporting the morseToText function for use in other files
module.exports = { morseToText };
