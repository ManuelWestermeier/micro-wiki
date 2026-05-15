#pragma once

#include <Arduino.h>

enum ButtonEvent
{
    BTN_NONE,
    BTN_CLICK,
    BTN_HOLD,
    BTN_DOUBLE_CLICK
};

ButtonEvent readButton();
