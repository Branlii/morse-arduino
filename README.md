# Morse Code Arduino Project

Un projet Arduino ESP32 qui permet de saisir du code morse via un bouton et de recevoir des messages traduits via WebSocket.

## Composants

- ESP32
- Bouton poussoir
- LED
- Grove Buzzer v1.2
- Écran LCD RGB 16x2
- Câbles de connexion

## Branchements ESP32

### Bouton Poussoir

- **Pin 18** → Bouton (autre côté du bouton vers GND)

### LED

- **Pin 13** → Anode LED (+)
- **GND** → Cathode LED (-)

### Grove Buzzer v1.2

- **Pin 5** → SIG
- **3.3V** → VCC
- **GND** → GND
- **NC** → Non connecté

### Écran LCD RGB

- **SDA** → Pin 21
- **SCL** → Pin 22
- **VCC** → 5V
- **GND** → GND

## Fonctionnement

1. **Saisie Morse**: Appuyez sur le bouton pour saisir des points (.) et tirets (-)

   - Appui court (< 300ms) = point
   - Appui long (≥ 300ms) = tiret

2. **Feedback Visuel**:

   - LED s'allume quand vous pouvez saisir une nouvelle lettre
   - LED s'éteint quand vous appuyez sur le bouton

3. **Feedback Sonore**:

   - Buzzer émet un son continu pendant l'appui du bouton

4. **Affichage**:

   - L'écran LCD affiche les messages reçus du serveur
   - Texte défile automatiquement si le message est long

5. **Communication**:
   - Envoie le code morse au serveur via WebSocket
   - Reçoit la traduction et l'affiche sur l'écran

## Configuration WiFi

Modifiez les paramètres WiFi dans le code :

```cpp
const char* ssid = "votre_ssid";
const char* password = "votre_mot_de_passe";
```

## Serveur WebSocket

Le projet se connecte à un serveur WebSocket sur `192.168.87.68:3000` pour la traduction du code morse.
