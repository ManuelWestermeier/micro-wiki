#include "loop.hpp"
#include "menu.hpp"
#include "article_browser.hpp"
#include "wifi_portal.hpp"
#include "shutdown.hpp"
#include "splash.hpp"
#include "../display.hpp"
#include "button.hpp"

extern bool isMenuOpen;
extern uint8_t menuState;

void loopUI()
{
    if (isMenuOpen)
    {
        menu();
        return;
    }

    switch (menuState)
    {
    case 0:
        renderArticleUI();
        break;
    case 1:
        renderEditUI();
        break;
    case 2:
        renderShutdownUI();
        break;
    }
}

void stateMachine()
{
    showSplash();
    while (true)
    {
        loopUI();
    }
}
