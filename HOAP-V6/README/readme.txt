# HOAP v6.0 - Hardware Orchestration Access Protocol
**Developer:** xxrandompaidenxx-hash

HOAP v6.0 is an advanced Arduino-based security gate system. It features a unique "Smart-Match" logic that allows for high-security physical fobs and convenient phone-based NFC access using a dynamic-bypass algorithm.

## 🚀 Features
- **EEPROM Storage**: Saves up to 40 Fobs and 10 Phones permanently.
- **Smart-Match Verification**: 
  - **Fobs**: 4-byte strict HEX matching.
  - **Phones**: 1-byte header matching (to bypass dynamic UID randomization).
- **Soft-Motion System**: Servo movement is synced with a buzzing audio frequency.
- **8th Bar Correction**: LCD output is optimized for hardware-specific display glitches.

## ⌨️ Command List (Serial Monitor @ 9600 Baud)
| Command | Action |
| :--- | :--- |
| `?` | Show Help Menu |
| `open` | Smooth lift with audio hum |
| `close` | Smooth close with audio hum |
| `status` | System health + Aperture Logo Easter Egg |
| `see` | HEX dump of all authorized UIDs |
| `add` | Enroll a new physical Fob |
| `phone` | Enroll a dynamic Phone NFC |
| `del` | Physical tap required to delete a UID |
| `shake` | Manual jitter/vibration test |
| `reset` | Recenter servo and clear LCD |

## 📦 Installation
1. Clone this repository or download the `.ino` file.
2. Connect hardware according to `Wiring.md`.
3. Flash the code to your Arduino Uno.
4. Use the `add` command via Serial Monitor to register your first device.