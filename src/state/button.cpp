#include "button.hpp"
#include "../config.hpp"

ButtonEvent readButton()
{
    if (digitalRead(PIN_BUTTON) != LOW)
        return BTN_NONE;

    uint32_t pressedAt = millis();

    while (digitalRead(PIN_BUTTON) == LOW)
    {
        if (millis() - pressedAt > 500)
        {
            while (digitalRead(PIN_BUTTON) == LOW)
                ;
            delay(50);
            return BTN_HOLD;
        }
    }

    delay(50);
    uint32_t releasedAt = millis();

    while (millis() - releasedAt < 250)
    {
        if (digitalRead(PIN_BUTTON) == LOW)
        {
            while (digitalRead(PIN_BUTTON) == LOW)
                ;
            delay(50);
            return BTN_DOUBLE_CLICK;
        }
    }

    return BTN_CLICK;
}
