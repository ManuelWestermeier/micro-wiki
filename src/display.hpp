#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

#define PIN_SCL 6
#define PIN_SDA 5

extern U8G2_SSD1306_72X40_ER_F_SW_I2C u8g2;
void initDisplay();
