#include <Arduino.h>

#include "config.hpp"
#include "display.hpp"
#include "data.hpp"
#include "state-machiene.hpp"

// ---------- Setup ----------
void setup()
{
  pinMode(PIN_BUTTON, INPUT_PULLDOWN);
  pinMode(PIN_LED, OUTPUT);

  initFS();
  initDisplay();

  stateMachine();
}

void loop()
{
}