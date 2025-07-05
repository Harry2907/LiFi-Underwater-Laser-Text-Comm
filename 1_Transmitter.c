#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define RELAY_PIN 13
#define LASER_PIN 12

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Keypad config
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {19, 18, 5, 23};
byte colPins[COLS] = {27, 14, 4, 15};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String inputBuffer = "";
String correctPassword = "1234";
bool authenticated = false;
bool textModeSelected = false;
int page = 0;  // 0 = first menu (1,2), 1 = second menu (3,4)

void setup() {
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LASER_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, HIGH);  // Default audio mode
  digitalWrite(LASER_PIN, LOW);

  lcd.setCursor(0, 0);
  lcd.print("Enter Password:");
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    if (!authenticated) {
      handlePassword(key);
    } else if (!textModeSelected) {
      handleModeSelection(key);
    } else {
      handleTextMenu(key);
    }
  }
}

void handlePassword(char key) {
  if (key == '*') {
    if (inputBuffer.length() > 0)
      inputBuffer.remove(inputBuffer.length() - 1);
  } else if (key == '#') {
    inputBuffer = "";
  } else {
    inputBuffer += key;
  }

  lcd.setCursor(0, 0);
  lcd.print("Enter Password:");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(inputBuffer);

  if (inputBuffer.length() == correctPassword.length()) {
    if (inputBuffer == correctPassword) {
      authenticated = true;
      inputBuffer = "";
      lcd.clear();
      lcd.print("A=Text  B=Audio");
    } else {
      lcd.clear();
      lcd.print("Wrong Password");
      delay(1500);
      lcd.clear();
      lcd.print("Enter Password:");
      inputBuffer = "";
    }
  }
}

void handleModeSelection(char key) {
  if (key == 'A') {
    textModeSelected = true;
    lcd.clear();
    showMenu();
  } else if (key == 'B') {
    lcd.clear();
    lcd.print("Audio Mode Ready");
    digitalWrite(RELAY_PIN, HIGH);  // Audio path
    delay(2000);
    lcd.clear();
    lcd.print("Enter Password:");
    authenticated = false;
    textModeSelected = false;
  }
}

void handleTextMenu(char key) {
  if (key == 'D') {  // Scroll DOWN
    page = min(page + 1, 1);
    showMenu();
  } else if (key == 'C') {  // Scroll UP
    page = max(page - 1, 0);
    showMenu();
  } else if (key == '1') {
    transmitText("HELLO ESP");
  } else if (key == '2') {
    transmitText("EMERGENCY");
  } else if (key == '3') {
    transmitText("SEND HELP");
  } else if (key == '4') {
    transmitText("ENEMY NEARBY");
  }
}

void showMenu() {
  lcd.clear();
  if (page == 0) {
    lcd.setCursor(0, 0);
    lcd.print("1:HELLO ESP");
    lcd.setCursor(0, 1);
    lcd.print("2:EMERGENCY");
  } else if (page == 1) {
    lcd.setCursor(0, 0);
    lcd.print("3:SEND HELP");
    lcd.setCursor(0, 1);
    lcd.print("4:ENEMY NEARBY");
  }
}

void transmitText(String text) {
  lcd.clear();
  lcd.print("Transmitting...");
  digitalWrite(RELAY_PIN, LOW);
  delay(500);
  
  sendText(text);
  
  digitalWrite(RELAY_PIN, HIGH);
  lcd.clear();
  lcd.print("Done!");
  delay(2000);
  lcd.clear();
  lcd.print("Enter Password:");
  
  authenticated = false;      // Force logout after transmit
  textModeSelected = false;
  page = 0;
}

void sendText(String text) {
  for (int i = 0; i < text.length(); i++) {
    sendChar(text[i]);
    delay(170);
  }
  digitalWrite(LASER_PIN, LOW);
}

void sendChar(char c) {
  for (int i = 7; i >= 0; i--) {
    bool bit = (c >> i) & 1;
    digitalWrite(LASER_PIN, bit);
    delay(150);
  }
}