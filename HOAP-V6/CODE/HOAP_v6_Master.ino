/**
 * Project: HOAP v6.0 (Hardware Orchestration Access Protocol)
 * Author: xxrandompaidenxx-hash
 * * Features:
 * - Smart-Match Logic (4-byte Fob / 1-byte Phone Bypass)
 * - 50 Slot EEPROM Memory (Permanent Storage)
 * - Soft-Motion Buzzing (Synced Pitch/Movement)
 * - 8th Bar LCD Glitch Correction
 */

#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <EEPROM.h>

// PINOUT
#define RST_PIN     9
#define SS_PIN      10
#define BUZZER_PIN  8
#define SERVO_PIN   6
#define EEPROM_ADDR 0 

// DATA STRUCTURE FOR EEPROM
struct SystemData {
  byte userDB[40][4];   // 40 Fob Slots (Strict 4-byte)
  byte phoneDB[10][4];  // 10 Phone Slots (1-byte bypass)
  int magicNumber;      // Boot check
} storage;

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo gateServo;

// SYSTEM FLAGS
bool enrollMode = false, deleteMode = false, phoneMode = false;
const int GATE_OPEN = 0, GATE_CLOSED = 90;

// LOGO EASTER EGG
byte logo[8] = {0b00000, 0b01110, 0b10101, 0b10011, 0b10101, 0b01110, 0b00000, 0b00000};

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, logo);
  
  gateServo.attach(SERVO_PIN);
  gateServo.write(GATE_CLOSED);

  // LOAD PERMANENT STORAGE
  EEPROM.get(EEPROM_ADDR, storage);
  if (storage.magicNumber != 1337) { 
    memset(storage.userDB, 0, sizeof(storage.userDB));
    memset(storage.phoneDB, 0, sizeof(storage.phoneDB));
    storage.magicNumber = 1337;
    saveToEEPROM();
  }

  Serial.println(F("HOAP v6.0 | SYSTEM ONLINE | USER: xxrandompaidenxx-hash"));
  lcd.clear(); lcd.print("READY");
}

void loop() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    handleCommand(cmd);
  }

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    if (enrollMode) enroll(storage.userDB, 40, false);
    else if (phoneMode) enroll(storage.phoneDB, 10, true);
    else if (deleteMode) wipeCard();
    else checkAccess();
    mfrc522.PICC_HaltA();
  }
}

void handleCommand(String cmd) {
  if (cmd == "?" || cmd == "help") {
    Serial.println(F("CMDS: open, close, status, see, add, phone, del, reset, shake, beep, test"));
  } 
  else if (cmd == "open") { softOpen(); }
  else if (cmd == "close") { buzzingSoftClose(); }
  else if (cmd == "status") { lcd.clear(); lcd.write(0); lcd.print(" ONLINE"); Serial.println(F("STATUS: SECURE")); }
  else if (cmd == "see") {
    for(int i=0; i<40; i++) if(storage.userDB[i][0] != 0) { Serial.print("FOB "); Serial.print(i); Serial.print(": "); dump(storage.userDB[i], 4); }
    for(int i=0; i<10; i++) if(storage.phoneDB[i][0] != 0) { Serial.print("PHN "); Serial.print(i); Serial.print(": "); dump(storage.phoneDB[i], 1); }
  }
  else if (cmd == "add") { enrollMode = true; Serial.println(F("MODE: ENROLL FOB")); }
  else if (cmd == "phone") { phoneMode = true; Serial.println(F("MODE: ENROLL PHONE")); }
  else if (cmd == "del") { deleteMode = true; Serial.println(F("TAP TO DELETE")); }
  else if (cmd == "reset") { gateServo.write(GATE_CLOSED); lcd.clear(); lcd.print("READY"); }
  else if (cmd == "shake") { for(int i=0; i<8; i++){ gateServo.write(85); delay(40); gateServo.write(90); delay(40); } }
  else if (cmd == "beep") { tone(BUZZER_PIN, 1000, 200); }
  else if (cmd == "test") { softOpen(); delay(1000); buzzingSoftClose(); }
}

void checkAccess() {
  bool found = false;
  byte tag[4];
  for(byte i=0; i<4; i++) tag[i] = mfrc522.uid.uidByte[i];

  // FOB SEARCH (4-BYTE)
  for(int i=0; i<40; i++) {
    if(storage.userDB[i][0] != 0 && memcmp(tag, storage.userDB[i], 4) == 0) found = true;
  }
  // PHONE SEARCH (1-BYTE)
  if(!found) {
    for(int i=0; i<10; i++) {
      if(storage.phoneDB[i][0] != 0 && tag[0] == storage.phoneDB[i][0]) found = true;
    }
  }

  if(found) {
    lcd.clear(); lcd.print("PASS");
    softOpen(); delay(3000); buzzingSoftClose();
    lcd.clear(); lcd.print("READY");
  } else {
    lcd.clear(); lcd.print("INTRUDR");
    Serial.print(F("INTRUDER UID: ")); dump(tag, 4);
    tone(BUZZER_PIN, 150, 800);
    delay(1000); lcd.clear(); lcd.print("READY");
  }
}

void softOpen() {
  for (int pos = GATE_CLOSED; pos >= GATE_OPEN; pos--) {
    gateServo.write(pos);
    tone(BUZZER_PIN, map(pos, GATE_CLOSED, GATE_OPEN, 800, 1400), 12);
    delay(15);
  }
  noTone(BUZZER_PIN);
}

void buzzingSoftClose() {
  for (int pos = GATE_OPEN; pos <= GATE_CLOSED; pos++) {
    gateServo.write(pos);
    tone(BUZZER_PIN, map(pos, GATE_OPEN, GATE_CLOSED, 300, 1000), 12);
    delay(20);
  }
  noTone(BUZZER_PIN);
}

void enroll(byte db[][4], int limit, bool isPhone) {
  for(int i=0; i<limit; i++) {
    if(db[i][0] == 0) {
      for(byte b=0; b<4; b++) db[i][b] = mfrc522.uid.uidByte[b];
      saveToEEPROM();
      Serial.println(F("AUTHORIZED"));
      tone(BUZZER_PIN, 1200, 200);
      enrollMode = false; phoneMode = false;
      return;
    }
  }
}

void wipeCard() {
  byte target = mfrc522.uid.uidByte[0];
  for(int i=0; i<40; i++) if(storage.userDB[i][0] == target) memset(storage.userDB[i], 0, 4);
  for(int i=0; i<10; i++) if(storage.phoneDB[i][0] == target) memset(storage.phoneDB[i], 0, 4);
  saveToEEPROM();
  Serial.println(F("UID PURGED"));
  tone(BUZZER_PIN, 200, 500);
  deleteMode = false;
}

void saveToEEPROM() { EEPROM.put(EEPROM_ADDR, storage); }
void dump(byte b[], int l) { for(int i=0; i<l; i++){ Serial.print(b[i], HEX); Serial.print(" "); } Serial.println(); }