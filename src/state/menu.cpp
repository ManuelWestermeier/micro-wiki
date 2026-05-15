#include "menu.hpp"
#include "button.hpp"
#include "../display.hpp"
#include <vector>
#include <string>

extern bool isMenuOpen;
extern uint8_t menuState;

void menu()
{
    static const std::vector<std::string> menuItems = {"Artikel", "Edit", "Aus"};

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_8x13_tr);

    for (size_t i = 0; i < menuItems.size(); i++)
    {
        if (i == menuState)
        {
            u8g2.drawBox(0, i * 13, 72, 13);
            u8g2.setDrawColor(0);
            u8g2.drawStr(2, (i + 1) * 13 - 2, menuItems[i].c_str());
            u8g2.setDrawColor(1);
        }
        else
        {
            u8g2.drawStr(2, (i + 1) * 13 - 2, menuItems[i].c_str());
        }
    }
    u8g2.sendBuffer();

    switch (readButton())
    {
    case BTN_CLICK:
        menuState = (menuState + 1) % menuItems.size();
        break;
    case BTN_HOLD:
        isMenuOpen = false;
        break;
    default:
        break;
    }

    delay(20);
}
