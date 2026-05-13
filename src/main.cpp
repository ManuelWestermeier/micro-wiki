#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#define PIN_SCL 6
#define PIN_SDA 5

U8G2_SSD1306_72X40_ER_F_SW_I2C u8g2(U8G2_R0, PIN_SCL, PIN_SDA, U8X8_PIN_NONE);

void setup()
{
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