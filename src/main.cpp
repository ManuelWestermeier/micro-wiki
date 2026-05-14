#include <Arduino.h>

#include "config.hpp"
#include "display.hpp"
#include "article-renderer.hpp"
#include "data.hpp"

// ---------- Setup ----------
void setup()
{
  pinMode(PIN_BUTTON, INPUT_PULLDOWN);
  pinMode(PIN_LED, OUTPUT);

  initFS();
  initDisplay();
}

// ---------- Loop ----------
void loop()
{
  renderArticle(readArticleBinary("/deutsch/grammatik_und_sprache/sprachsystem.bin"));
}