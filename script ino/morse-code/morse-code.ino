#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include "rgb_lcd.h"
#include <WebSocketsClient.h>

WebSocketsClient webSocket;

const int buttonPin = 18;
const int ledPin = 13; // Changed to pin 13 - safer for ESP32, doesn't conflict with WiFi
const int buzzerPin = 5; // Grove buzzer v1.2 SIG pin
rgb_lcd lcd;

unsigned long pressStartTime = 0;
unsigned long lastInputTime = 0;
String morseCode = "";

bool buttonPressed = false;

String morseInput = "";

int maxLetter = 64;

// Array to store morse code segments
String morseSending[64] = {};
int morseCount = 0;

String receivedMessage = ""; // To store the message from the server
unsigned long scrollTime = 0; // For tracking scroll timing
int scrollPosition = 0; // Current scroll position

// Ultrasonic sensor variables
bool buzzerActive = false;
unsigned long buzzerStartTime = 0;
const unsigned long buzzerDuration = 100; // Sound duration in ms

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_TEXT) {
    String reply = String((char*)payload);
    // reply is the translated text from server
    Serial.println("Server says: " + reply);
    
    // Save the received message and reset scroll position
    receivedMessage = reply;
    scrollPosition = 0;
    scrollTime = millis();
    
    // Display the message on the LCD's second line
    lcd.setCursor(0, 1);
    lcd.print("                "); // Clear the second line
    lcd.setCursor(0, 1);
    
    // If the message is shorter than or equal to 16 characters, just display it
    if (receivedMessage.length() <= 16) {
      lcd.print(receivedMessage);
    } else {
      // Display the first 16 characters of the message
      lcd.print(receivedMessage.substring(0, 16));
    }
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
  Serial.begin(9600);
  delay(1000);

  Serial.println("Booting...");

  // Initialize the LED
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Start with LED off
  
  // Test LED blink to verify it works
  digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);

  // Initialize the LCD
  lcd.begin(16, 2); // Assuming 16x2 LCD - adjust if different
  lcd.setRGB(255, 255, 255); // Set backlight to white
  lcd.print("Incoming message");
  
  delay(100);  // Small delay for stability

  // Initialize buzzer
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  setupWiFi();
}

void loop() {
  webSocket.loop();

  // Handle scrolling text if the message is longer than 16 characters
  if (receivedMessage.length() > 16) {
    // Scroll every 500 milliseconds
    if (millis() - scrollTime > 500) {
      scrollTime = millis();
      scrollPosition++;
      
      // Reset scroll position when we've scrolled through the entire message
      if (scrollPosition > receivedMessage.length()) {
        scrollPosition = 0;
      }
      
      // Update the display with the scrolled text
      lcd.setCursor(0, 1);
      lcd.print("                "); // Clear the line
      lcd.setCursor(0, 1);
      
      // Calculate the end position for substring
      int endPosition = scrollPosition + 16;
      if (endPosition > receivedMessage.length()) {
        // If we're near the end, show what we can and wrap around
        String displayText = receivedMessage.substring(scrollPosition);
        // Pad with spaces if needed
        while (displayText.length() < 16) {
          displayText += " ";
        }
        lcd.print(displayText);
      } else {
        // Normal case - show 16 characters from the current position
        lcd.print(receivedMessage.substring(scrollPosition, endPosition));
      }
    }
  }

  bool buttonState = digitalRead(buttonPin);

  // Button press detection if button is pressed and not already pressed
  if (buttonState == 1 && buttonPressed == false) {
    pressStartTime = millis();
    buttonPressed = true;
    digitalWrite(ledPin, LOW); // Turn off LED when user starts pressing
    
    // Trigger buzzer sound
    digitalWrite(buzzerPin, HIGH);
    buzzerActive = true;
    buzzerStartTime = millis();
    
    Serial.println(pressStartTime);
  }

  // Button release detection if button is released and already pressed
  if (buttonState == 0 && buttonPressed == true) {
    unsigned long currentTime = millis();
    unsigned long pressDuration = currentTime - pressStartTime;
    buttonPressed = false;
    
    // Turn off buzzer when button is released
    digitalWrite(buzzerPin, LOW);

    // Determine if it's a dot or dash
    if (pressDuration < 300) {
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
  if (morseInput.length() > 0 && (millis() - lastInputTime > 700)) {
    if (morseCount < maxLetter) { // Ensure we don't exceed array bounds
      morseSending[morseCount] = morseInput;
      morseCount++;
      Serial.print("Added to array: ");
      Serial.println(morseInput);
    }
    morseInput = "";
    lastInputTime = millis(); // Reset timer to avoid triggering the 5-second condition immediately
    digitalWrite(ledPin, HIGH); // Turn on LED - user can tap new letter
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
