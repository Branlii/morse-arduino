#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include "rgb_lcd.h"
#include <WebSocketsClient.h>

WebSocketsClient webSocket;

const int buttonPin = 18;
rgb_lcd lcd;

unsigned long pressStartTime = 0;
unsigned long lastInputTime = 0;
String morseCode = "";

bool buttonPressed = false;

String morseInput = "";

// Array to store morse code segments
String morseSending[32] = {};
int morseCount = 0;

const char* ssid = "ESP32-Morse";
const char* password = "12345678";

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_TEXT) {
    String reply = String((char*)payload);
    // reply is the translated text from server
    Serial.println("Server says: " + reply);
    // you could also re-encode to morse here
  }
}

void setupWebSocket() {
  Serial.println("Trying to connect to 192.168.87.68:3000");
  webSocket.begin("192.168.87.68", 3000, "/");
  webSocket.onEvent(webSocketEvent);
}

void setupWiFi() {
  WiFi.mode(WIFI_STA);  // Set to station mode

  const char* ssid = "ss";             // Replace with your hotspot SSID
  const char* password = "ez1234567";  // Replace with your actual hotspot password

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("Connected to WiFi.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Try connecting to websocket after being successfully connected to the WiFi
    setupWebSocket();
  } else {
    Serial.println("");
    Serial.println("Failed to connect to WiFi.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Booting...");

  delay(100);  // Small delay for stability

  setupWiFi();
}

void loop() {
  webSocket.loop();

  bool buttonState = digitalRead(buttonPin);

  // Button press detection if button is pressed and not already pressed
  if (buttonState == 1 && buttonPressed == false) {
    pressStartTime = millis();
    buttonPressed = true;
    Serial.println(pressStartTime);
  }

  // Button release detection if button is released and already pressed
  if (buttonState == 0 && buttonPressed == true) {
    unsigned long currentTime = millis();
    unsigned long pressDuration = currentTime - pressStartTime;
    buttonPressed = false;

    // Determine if it's a dot or dash
    if (pressDuration < 500) {
      morseInput += ".";
      Serial.println("Detected: DOT");
    } else {
      morseInput += "-";
      Serial.println("Detected: DASH");
    }

    lastInputTime = currentTime;
    Serial.print("Current morse input: ");
    Serial.println(morseInput);
  }

  // Check for 2 seconds pause
  if (morseInput.length() > 0 && (millis() - lastInputTime > 2000)) {
    if (morseCount < 10) { // Ensure we don't exceed array bounds
      morseSending[morseCount] = morseInput;
      morseCount++;
      Serial.print("Added to array: ");
      Serial.println(morseInput);
    }
    morseInput = "";
    lastInputTime = millis(); // Reset timer to avoid triggering the 5-second condition immediately
  }

  // Check for 5 seconds pause and send through socket
  if (millis() - lastInputTime > 5000 && morseCount > 0) {
    // Prepare JSON string to send morse code array
    String jsonData = "[";
    for (int i = 0; i < morseCount; i++) {
      jsonData += "\"" + morseSending[i] + "\"";
      if (i < morseCount - 1) {
        jsonData += ",";
      }
    }
    jsonData += "]";
    
    // Send via WebSocket
    webSocket.sendTXT(jsonData);
    Serial.print("Sent morse code array via WebSocket: ");
    Serial.println(jsonData);
    
    // Reset the array and counter
    morseCount = 0;
    for (int i = 0; i < 10; i++) {
      morseSending[i] = "";
    }
  }
}
