#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

const int buttonPin = 18;

String morseInput = "";
unsigned long pressStartTime = 0;
unsigned long lastInputTime = 0;

bool buttonPressed = false;

struct MorseCode {
  const char* code;
  char letter;
};

// Morse code table
MorseCode morseTable[] = {
  {".-", 'A'},   {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'},
  {".", 'E'},    {"..-.", 'F'}, {"--.", 'G'},  {"....", 'H'},
  {"..", 'I'},   {".---", 'J'}, {"-.-", 'K'},  {".-..", 'L'},
  {"--", 'M'},   {"-.", 'N'},   {"---", 'O'},  {".--.", 'P'},
  {"--.-", 'Q'}, {".-.", 'R'},  {"...", 'S'},  {"-", 'T'},
  {"..-", 'U'},  {"...-", 'V'}, {".--", 'W'},  {"-..-", 'X'},
  {"-.--", 'Y'}, {"--..", 'Z'}
};
const int tableSize = sizeof(morseTable) / sizeof(MorseCode);

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);

  lcd.begin(16, 2);
  lcd.setRGB(0, 255, 0);
  lcd.print("Morse Decoder");
  delay(1500);
  lcd.clear();
}

void loop() {
  bool buttonState = digitalRead(buttonPin);

  // Button press detection if button is pressed and not already pressed
  if (buttonState == 1 && buttonPressed == false) {
    Serial.print("Button pressed - timing started at: ");
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

  // Check for 2 seconds pause => decode
  if (morseInput.length() > 0 && (millis() - lastInputTime > 2000)) {
    char decoded = decodeMorse(morseInput);
    if (decoded != '?') {
      lcd.print(decoded);
      Serial.print("Decoded: ");
      Serial.println(decoded);
    } else {
      lcd.print("#");  // Unknown
      Serial.println("Decoded: Unknown");
    }
    morseInput = "";
  }
}

char decodeMorse(String code) {
  for (int i = 0; i < tableSize; i++) {
    if (code == morseTable[i].code) {
      return morseTable[i].letter;
    }
  }
  return '?';
}
