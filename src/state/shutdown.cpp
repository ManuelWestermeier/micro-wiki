#include "shutdown.hpp"
#include "button.hpp"
#include "../config.hpp"
#include "../display.hpp"
#include <esp_sleep.h>
#include <WiFi.h>

extern bool isMenuOpen;
extern uint8_t menuState;

void renderShutdownUI()
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_8x13_tr);
    u8g2.drawStr(4, 15, "Schlafen?");
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(2, 28, "Hold=Ja");
    u8g2.drawStr(2, 38, "2x=Zuruck");
    u8g2.sendBuffer();

    switch (readButton())
    {
    case BTN_HOLD:
    {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_8x13_tr);
        u8g2.drawStr(20, 24, "Tschuss!");
        u8g2.sendBuffer();
        delay(800);

        u8g2.clearBuffer();
        u8g2.sendBuffer();
        u8g2.setPowerSave(1);

        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_OFF);

        esp_deep_sleep_enable_gpio_wakeup(1ULL << PIN_BUTTON, ESP_GPIO_WAKEUP_GPIO_LOW);
        esp_deep_sleep_start();
        break;
    }
    case BTN_DOUBLE_CLICK:
        isMenuOpen = true;
        menuState = 0;
        break;
    default:
        break;
    }

    delay(20);
}
