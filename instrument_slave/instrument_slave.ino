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
#define LCD_D5_PIN 7
#define LCD_D6_PIN 3
#define LCD_D7_PIN 2

#define LCD_ROWS 2
#define LCD_COLS 16

#define CMD_SET_NAME  0x1
#define CMD_GET_STATE 0x2

LiquidCrystal lcd(LCD_RS_PIN, LCD_EN_PIN, LCD_D4_PIN, LCD_D5_PIN,
                  LCD_D6_PIN, LCD_D7_PIN);

void setup() {
  lcd.begin(LCD_COLS, LCD_ROWS);
  TinyWireS.begin(I2C_SLAVE_ADDR);
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(1000);
  lcd.clear();
}

void loop() {
  if (TinyWireS.available()) {
    lcd.setCursor(0, 1);
    byte byteRcvd = TinyWireS.receive();
    lcd.print(byteRcvd);
    delay(1000);
    switch (byteRcvd) {
      case CMD_SET_NAME:
        setName();
        break;
      case CMD_GET_STATE:
        getState();
        break;
      default:
        break;
    }
  }
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

void getState() {
  TinyWireS.send(10);
}

