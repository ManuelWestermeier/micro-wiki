#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <vector>

#define PIN_SCL 6
#define PIN_SDA 5
#define PIN_LED 8
#define PIN_BUTTON 9

U8G2_SSD1306_72X40_ER_F_SW_I2C u8g2(U8G2_R0, PIN_SCL, PIN_SDA, U8X8_PIN_NONE);

using namespace std;

struct Item
{
  String typ;
  vector<String> daten;
};

vector<Item> content = {
    {"H", {"Demokratietheorie & Rechtsstaatsprinzipien"}},
    {"T", {"Demokratie (griech. demos=Volk, kratos=Herrschaft) ist das einzige Regierungssystem, das regelmäßige Machtablösung ohne Gewalt ermöglicht."}},
    {"LI", {"Direkte Demokratie: Bürger entscheiden direkt (Schweiz, Athen)", "Repräsentative Demokratie: Vertreter entscheiden (DE, USA)", "Konstitutionelle Monarchie: Monarch eingeschränkt (GB, Spanien)", "Westminster-System: Regierung vom Parlament abhängig", "Präsidentielles System: Präsident unabhängig (USA)", "Verhältniswahlrecht: Stimmen → Sitze", "Mehrheitswahlrecht: Gewinner-nimmt-alles", "Föderalismus: Bund vs. Länder"}},
    {"H", {"Internationale Rechtssysteme"}},
    {"LI", {"Common Law: Richterrecht (GB, USA)", "Civil Law: Kodifiziert (DE, FR)", "Scharia: religiöses Recht", "Völkerrecht: zwischen Staaten", "ICC: Den Haag, Kriegsverbrechen", "WTO: Handelsstreitigkeiten"}}};

struct Line
{
  String text;
  uint8_t type; // 0=H,1=T,2=LI
};

vector<Line> rendered;

static int offsetY = 0;

// dynamische Breite (Pixel → Zeichen)
int getMaxChars(uint8_t type)
{
  if (type == 0)
    return 9; // große Schrift
  if (type == 2)
    return 11; // LI
  return 12;   // T
}

// intelligenter Wrap mit Wortverschiebung + harter Trennung
vector<String> wrapTextSmart(String text, int maxChars)
{
  vector<String> lines;

  while (text.length() > 0)
  {
    if (text.length() <= maxChars)
    {
      lines.push_back(text);
      break;
    }

    int cut = text.lastIndexOf(' ', maxChars);

    if (cut > 0)
    {
      lines.push_back(text.substring(0, cut));
      text = text.substring(cut + 1);
    }
    else
    {
      // hart trennen mit "-"
      lines.push_back(text.substring(0, maxChars - 1) + "-");
      text = text.substring(maxChars - 1);
    }
  }

  return lines;
}

void buildRenderBuffer()
{
  rendered.clear();

  for (auto &item : content)
  {

    if (item.typ == "H")
    {
      auto lines = wrapTextSmart(item.daten[0], getMaxChars(0));
      for (auto &l : lines)
        rendered.push_back({l, 0});
    }

    if (item.typ == "T")
    {
      auto lines = wrapTextSmart(item.daten[0], getMaxChars(1));
      for (auto &l : lines)
        rendered.push_back({l, 1});
    }

    if (item.typ == "LI")
    {
      for (auto &entry : item.daten)
      {
        auto lines = wrapTextSmart(entry, getMaxChars(2));
        if (!lines.empty())
          lines[0] = "> " + lines[0];
        for (auto &l : lines)
          rendered.push_back({l, 2});
      }
    }

    // fester Item-Abstand 15px
    rendered.push_back({"", 3});
  }
}

void setFont(uint8_t type)
{
  if (type == 0)
    u8g2.setFont(u8g2_font_7x13B_tf); // groß
  else if (type == 1)
    u8g2.setFont(u8g2_font_6x12_tf);
  else
    u8g2.setFont(u8g2_font_6x10_tf);
}

int getLineHeight(uint8_t type)
{
  if (type == 0)
    return 14;
  if (type == 1)
    return 12;
  if (type == 2)
    return 10;
  return 0;
}

int getSpacing(uint8_t type, bool isNextLI)
{
  if (type == 0)
    return 5;
  if (type == 1)
    return 2;
  if (type == 2)
    return isNextLI ? 8 : 2;
  return 15; // Item Abstand
}

void draw()
{
  u8g2.clearBuffer();

  int y = offsetY;

  for (int i = 0; i < rendered.size(); i++)
  {
    auto &line = rendered[i];

    if (line.type == 3)
    {
      y += 15;
      continue;
    }

    setFont(line.type);
    int h = getLineHeight(line.type);

    if (y > -h && y < 48)
    {
      u8g2.drawStr(0, y + h, line.text.c_str());
    }

    bool nextIsLI = false;
    if (i + 1 < rendered.size())
    {
      nextIsLI = (rendered[i + 1].type == 2);
    }

    y += h + getSpacing(line.type, nextIsLI);
  }

  u8g2.sendBuffer();
}

void setup()
{
  pinMode(PIN_BUTTON, INPUT_PULLDOWN);
  pinMode(PIN_LED, OUTPUT);

  Wire.begin(PIN_SDA, PIN_SCL);
  u8g2.begin();

  buildRenderBuffer();
  draw();
}

void loop()
{
  if (digitalRead(PIN_BUTTON) == LOW)
  {
    unsigned long start = millis();
    bool shortPress = true;

    while (digitalRead(PIN_BUTTON) == LOW)
    {
      if (millis() - start > 400)
      {
        offsetY -= 3;
        shortPress = false;
        draw();
        delay(30);
      }
    }

    if (shortPress)
    {
      offsetY += 10;
      draw();
    }
  }
}