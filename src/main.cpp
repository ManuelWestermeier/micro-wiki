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

void setup()
{
  pinMode(PIN_BUTTON, INPUT_PULLDOWN);
  pinMode(PIN_LED, OUTPUT);

  Wire.begin(PIN_SDA, PIN_SCL);
  u8g2.begin();
}

static int offsetY = 0;

void draw()
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_t_cyrillic);
  for (int i = 0; i < data.size(); i++)
  {
    u8g2.drawStr(0, 10 + i * 10 + offsetY, data[i].c_str());
  }
  u8g2.sendBuffer();
}

void loop()
{
  if (digitalRead(PIN_BUTTON) == LOW)
  {
    auto currentTime = millis();
    bool down = true;
    while (digitalRead(PIN_BUTTON) == LOW)
    {
      if (millis() - currentTime > 500)
      {
        offsetY -= 5;
        down = false;
        draw();
      }
    }
    if (down)
    {
      offsetY += 5;
      draw();
    }
  }
}