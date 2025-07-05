#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#define LDR_PIN 14
#define BIT_DURATION 170
#define CHAR_SPACING 250

LiquidCrystal_I2C lcd(0x27, 16, 2);
String currentMessage = "";
String correctPassword = "1234";
String inputBuffer = "";
bool messageAvailable = false;
bool awaitingPassword = false;

// Keypad setup
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

void setup() {
  Serial.begin(9600);
  pinMode(LDR_PIN, INPUT);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Enter Password:");
  lcd.setCursor(0, 1);
}

void loop() {
  if (!messageAvailable) {
    if (digitalRead(LDR_PIN) == LOW) {
      delay(BIT_DURATION / 2);
      currentMessage = "";

      while (digitalRead(LDR_PIN) == LOW) {
        String bitString = "";

        for (int i = 0; i < 8; i++) {
          int bit = digitalRead(LDR_PIN) == LOW ? 1 : 0;
          bitString += String(bit);
          delay(BIT_DURATION);
        }

        Serial.print("Received: ");
        Serial.println(bitString);

        char receivedChar = decode(bitString);
        Serial.print("Char: ");
        Serial.println(receivedChar);

        currentMessage += receivedChar;
        delay(CHAR_SPACING);
      }

      messageAvailable = true;
      lcd.setCursor(15, 0);  // Right corner of first row
      lcd.print("!");
    }
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
          lcd.print("Msg:");
          lcd.setCursor(0, 1);
          lcd.print(currentMessage.substring(0, 16));
          delay(4000);
        } else {
          lcd.print("Wrong Password");
          delay(2000);
        }

        lcd.clear();
        lcd.print("Enter Password:");
        lcd.setCursor(0, 1);
        lcd.print("                ");
        inputBuffer = "";
        currentMessage = "";
        messageAvailable = false;
      }
    }
  }
}

char decode(String bits) {
  if (bits == "10010000") return 'H';
  else if (bits == "10001011" || bits == "10001010" || bits == "10100011") return 'E';
  else if (bits == "10011000" || bits == "11000010") return 'L';
  else if (bits == "10011111" || bits == "10011110") return 'O';
  else if (bits == "10000000" || bits == "10100000") return 'P';
  else if (bits == "10100110" || bits == "01001111") return 'S';
  else if (bits == "10100100") return 'R';
  else if (bits == "10000010") return 'A';
  else if (bits == "10101000") return 'T';
  else if (bits == "10000110") return 'C';
  else if (bits == "10010110") return 'K';
  else return '?';
}