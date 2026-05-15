#include "splash.hpp"
#include "../display.hpp"

void showSplash()
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_10x20_tr);
    u8g2.drawStr(16, 30, "W2GO");
    u8g2.sendBuffer();
    delay(1000);

    u8g2.clearBuffer();
    u8g2.drawBox(0, 0, 72, 40);
    u8g2.sendBuffer();
    delay(80);

    u8g2.clearBuffer();
    u8g2.sendBuffer();
    delay(80);
}
