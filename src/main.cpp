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
  uint8_t type; // 0=H,1=T,2=LI,3=Spacer
};

vector<Line> rendered;

int offsetY = 0;
unsigned long lastScroll = 0;

enum ScrollMode
{
  DOWN,
  STOP1,
  UP,
  STOP2
};
ScrollMode mode = STOP1;

// ---------- Umlaut-Ersatz ----------
String normalize(String s)
{
  s.replace("ä", "ae");
  s.replace("ö", "oe");
  s.replace("ü", "ue");
  s.replace("Ä", "Ae");
  s.replace("Ö", "Oe");
  s.replace("Ü", "Ue");
  s.replace("ß", "ss");
  return s;
}

// ---------- Font ----------
void setFont(uint8_t t)
{
  if (t == 0)
    u8g2.setFont(u8g2_font_7x13B_tf);
  else if (t == 1)
    u8g2.setFont(u8g2_font_6x12_tf);
  else
    u8g2.setFont(u8g2_font_6x10_tf);
}

int getFH(uint8_t t)
{
  if (t == 0)
    return 14;
  if (t == 1)
    return 12;
  if (t == 2)
    return 10;
  return 0;
}

int getMaxChars(uint8_t t)
{
  if (t == 0)
    return 9;
  if (t == 2)
    return 11;
  return 12;
}

// ---------- Smart Wrap ----------
vector<String> wrap(String txt, int maxC)
{
  vector<String> out;
  txt = normalize(txt);

  while (txt.length())
  {
    if (txt.length() <= maxC)
    {
      out.push_back(txt);
      break;
    }

    int cut = txt.lastIndexOf(' ', maxC);

    if (cut > 0)
    {
      out.push_back(txt.substring(0, cut));
      txt = txt.substring(cut + 1);
    }
    else
    {
      out.push_back(txt.substring(0, maxC - 1) + "-");
      txt = txt.substring(maxC - 1);
    }
  }
  return out;
}

// ---------- Build ----------
void build()
{
  rendered.clear();

  for (auto &it : content)
  {

    if (it.typ == "H")
    {
      auto l = wrap(it.daten[0], getMaxChars(0));
      for (auto &s : l)
        rendered.push_back({s, 0});
    }

    if (it.typ == "T")
    {
      auto l = wrap(it.daten[0], getMaxChars(1));
      for (auto &s : l)
        rendered.push_back({s, 1});
    }

    if (it.typ == "LI")
    {
      for (auto &e : it.daten)
      {
        auto l = wrap(e, getMaxChars(2));
        if (!l.empty())
          l[0] = "> " + l[0];
        for (auto &s : l)
          rendered.push_back({s, 2});
      }
    }

    rendered.push_back({"", 3});
  }
}

// ---------- Draw ----------
void draw()
{
  u8g2.clearBuffer();

  int y = offsetY;

  for (int i = 0; i < rendered.size(); i++)
  {
    auto &ln = rendered[i];

    if (ln.type == 3)
    {
      y += 15;
      continue;
    }

    setFont(ln.type);
    int fh = getFH(ln.type);

    if (y > -fh && y < 48)
    {
      u8g2.drawStr(0, y + fh, ln.text.c_str());
    }

    int spacing = 2;
    if (ln.type == 0)
      spacing = 5;
    if (ln.type == 2)
    {
      if (i + 1 < rendered.size() && rendered[i + 1].type == 2)
        spacing = 8;
      else
        spacing = 2;
    }

    y += fh + spacing;
  }

  u8g2.sendBuffer();
}

// ---------- Scroll Step ----------
int getCurrentStep()
{
  for (auto &l : rendered)
  {
    if (l.type != 3)
    {
      return getFH(l.type) / 3.5;
    }
  }
  return 3;
}

// ---------- Setup ----------
void setup()
{
  pinMode(PIN_BUTTON, INPUT_PULLDOWN);
  Wire.begin(PIN_SDA, PIN_SCL);
  u8g2.begin();

  build();
  draw();
}

// ---------- Loop ----------
void loop()
{
  static bool lastBtn = HIGH;

  bool btn = digitalRead(PIN_BUTTON);

  // Klick erkannt
  if (lastBtn == HIGH && btn == LOW)
  {
    if (mode == DOWN)
      mode = STOP1;
    else if (mode == STOP1)
      mode = UP;
    else if (mode == UP)
      mode = STOP2;
    else
      mode = DOWN;

    delay(200);
  }

  lastBtn = btn;

  int step = getCurrentStep();

  if (millis() - lastScroll > 80)
  {
    lastScroll = millis();

    if (mode == DOWN)
    {
      offsetY -= step;
      draw();
    }
    else if (mode == UP)
    {
      offsetY += step;
      draw();
    }
  }
}