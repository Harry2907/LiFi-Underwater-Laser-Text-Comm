#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <AESLib.h>

#define LDR_PIN 14
#define BIT_DURATION 3333  // in microseconds, for 300 bps
#define CHAR_BITS 8
#define MAX_MSG_LEN 64

LiquidCrystal_I2C lcd(0x27, 16, 2);
AESLib aesLib;

String encryptedBuffer = "";
String correctPassword = "1234";
String inputBuffer = "";
bool messageAvailable = false;
bool authenticated = false;

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {19, 18, 5, 23};
byte colPins[COLS] = {27, 14, 4, 15};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

byte aesKey[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45};
byte aesIv[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void setup() {
  Serial.begin(9600);
  pinMode(LDR_PIN, INPUT);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Waiting Msg...");
}

void loop() {
  if (!messageAvailable && detectStart()) {
    encryptedBuffer = "";
    while (true) {
      char ch = receiveChar();
      if (ch == '\0') break;
      encryptedBuffer += ch;
    }
    messageAvailable = true;
    lcd.clear();
    lcd.print("Msg Received!");
    delay(1000);
    lcd.clear();
    lcd.print("Enter Password:");
  }

  if (messageAvailable) {
    char key = keypad.getKey();
    if (key) {
      if (key == '*') {
        if (inputBuffer.length() > 0) inputBuffer.remove(inputBuffer.length() - 1);
      } else if (key == '#') {
        inputBuffer = "";
      } else {
        inputBuffer += key;
      }

      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(inputBuffer);

      if (inputBuffer.length() == correctPassword.length()) {
        lcd.clear();
        if (inputBuffer == correctPassword) {
          char decrypted[MAX_MSG_LEN];
          int len = aesLib.decrypt((char*)encryptedBuffer.c_str(), encryptedBuffer.length(), decrypted, aesKey, aesIv);
          decrypted[len] = '\0';

          lcd.setCursor(0, 0);
          lcd.print("Msg:");
          lcd.setCursor(0, 1);
          lcd.print(String(decrypted).substring(0, 16));
          delay(4000);
        } else {
          lcd.print("Wrong Password");
          delay(2000);
        }

        lcd.clear();
        lcd.print("Waiting Msg...");
        inputBuffer = "";
        encryptedBuffer = "";
        messageAvailable = false;
      }
    }
  }
}

bool detectStart() {
  return digitalRead(LDR_PIN) == LOW;
}

char receiveChar() {
  byte val = 0;
  for (int i = 7; i >= 0; i--) {
    int bit = digitalRead(LDR_PIN) == LOW ? 1 : 0;
    val |= (bit << i);
    delayMicroseconds(BIT_DURATION);
  }

  // Assume 0x00 as termination signal
  if (val == 0x00) return '\0';
  return (char)val;
}
