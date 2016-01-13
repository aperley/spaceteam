/*
  The circuit:
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 */

#include <LiquidCrystal.h>
#include "TinyWireS.h"

#define I2C_SLAVE_ADDR 0x10
#define LCD_RS_PIN 0
#define LCD_EN_PIN 1
#define LCD_D4_PIN 5
#define LCD_D5_PIN 9
#define LCD_D6_PIN 3
#define LCD_D7_PIN 2

#define INPUT_PIN 7

#define LCD_ROWS 2
#define LCD_COLS 16

#define CMD_SET_NAME  0x1
#define CMD_GET_STATE 0x2

LiquidCrystal lcd(LCD_RS_PIN, LCD_EN_PIN, LCD_D4_PIN, LCD_D5_PIN,
                  LCD_D6_PIN, LCD_D7_PIN);

void setup() {
  pinMode(INPUT_PIN, INPUT);
  
  lcd.begin(LCD_COLS, LCD_ROWS);
  TinyWireS.begin(I2C_SLAVE_ADDR);
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(1000);
  lcd.clear();
}

void loop() {
  int analogVal = analogRead(INPUT_PIN);
  byte inputState = analogVal * 5 / 1024;
  lcd.setCursor(0, 1);
  lcd.print(inputState);
  
  if (TinyWireS.available()) {
    byte byteRcvd = TinyWireS.receive();
    switch (byteRcvd) {
      case CMD_SET_NAME:
        setName();
        break;
      case CMD_GET_STATE:
        getState(inputState);
        break;
      default:
        break;
    }
  }

  delay(10);
}

void setName() {
  lcd.clear();
  int col = 0;  
  while (TinyWireS.available()) {
    char byteRcvd = TinyWireS.receive();
    lcd.setCursor(col, 0);
    lcd.print(byteRcvd);
    col++;
    if (col == LCD_COLS) {
      col = 0;
    }
  }
}

void getState(byte inputState) {
  TinyWireS.send(inputState);
}

