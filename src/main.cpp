#include <Arduino.h>

#include "config.hpp"
#include "display.hpp"
#include "article-renderer.hpp"
#include "data.hpp"

Article content = {
    {H, {"Demokratietheorie & Rechtsstaatsprinzipien"}},
    {T, {"Demokratie (griech. demos=Volk, kratos=Herrschaft) ist das einzige Regierungssystem, das regelmäßige Machtabloesung ohne Gewalt ermoeglicht."}},
    {LI, {"Direkte Demokratie: Buerger entscheiden direkt (Schweiz, Athen)", "Repraesentative Demokratie: Vertreter entscheiden (DE, USA)", "Konstitutionelle Monarchie: Monarch eingeschraenkt (GB, Spanien)", "Westminster-System: Regierung vom Parlament abhaengig", "Praesidentielles System: Praesident unabhaengig (USA)", "Verhaeltniswahlrecht: Stimmen → Sitze", "Mehrheitswahlrecht: Gewinner-nimmt-alles", "Foederalismus: Bund vs. Laender"}},
    {H, {"Internationale Rechtssysteme"}},
    {LI, {"Common Law: Richterrecht (GB, USA)", "Civil Law: Kodifiziert (DE, FR)", "Scharia: religioeses Recht", "Voelkerrecht: zwischen Staaten", "ICC: Den Haag, Kriegsverbrechen", "WTO: Handelsstreitigkeiten"}}};

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
  renderArticle(content);
}