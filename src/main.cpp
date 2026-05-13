#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#include <vector>

#define PIN_SCL 6
#define PIN_SDA 5

#define PIN_LED 8
#define PIN_BUTTON 9

U8G2_SSD1306_72X40_ER_F_SW_I2C u8g2(U8G2_R0, PIN_SCL, PIN_SDA, U8X8_PIN_NONE);

using namespace std;

vector<String> data = {"geschichte", "geographie", "politik_gesellschaft", "sport", "kultur", "wissenschaft_technik", "sonstiges"};

void IRAM_ATTR onButtonInterrupt()
{
  digitalWrite(PIN_LED, !digitalRead(PIN_LED));
}

void setup()
{
  pinMode(PIN_BUTTON, INPUT_PULLDOWN);
  pinMode(PIN_LED, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON), onButtonInterrupt, RISING);

  Wire.begin(PIN_SDA, PIN_SCL);

  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(0, 12, "Hello");
  u8g2.sendBuffer();
}

void loop()
{
}