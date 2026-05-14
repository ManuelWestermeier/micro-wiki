#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include <vector>

#include "config.hpp"
#include "display.hpp"

using namespace std;

enum Type
{
    H,
    T,
    LI
};

struct Item
{
    Type typ;
    vector<String> daten;
};

struct Line
{
    String text;
    uint8_t type;
};

#define Article vector<Item>

struct ArticleRenderer
{
    Article content;

    vector<Line> rendered;

    int offsetY = 0;
    int minOffsetY = 0;

    unsigned long lastScroll = 0;

    enum ScrollMode
    {
        DOWN,
        STOP1,
        UP,
        STOP2
    };
    ScrollMode mode = STOP1;

    unsigned long pressStart = 0;
    bool isHolding = false;
    const int HOLD_THRESHOLD = 300;

    ArticleRenderer(const Article &c) : content(c)
    {
        build();
        calculateHeight();
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

    // ---------- Wrap ----------
    vector<String> wrap(String txt, int maxC)
    {
        vector<String> out;

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
            if (it.typ == H)
            {
                auto l = wrap(it.daten[0], getMaxChars(0));
                for (auto &s : l)
                    rendered.push_back({s, 0});
            }

            if (it.typ == T)
            {
                auto l = wrap(it.daten[0], getMaxChars(1));
                for (auto &s : l)
                    rendered.push_back({s, 1});
            }

            if (it.typ == LI)
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

        // Ende
        rendered.push_back({"-- ENDE --", 0});
        rendered.push_back({"", 3});
        rendered.push_back({"", 3});
    }

    // ---------- Höhe berechnen ----------
    void calculateHeight()
    {
        int y = 0;

        for (int i = 0; i < rendered.size(); i++)
        {
            auto &ln = rendered[i];

            if (ln.type == 3)
            {
                y += 15;
                continue;
            }

            int fh = getFH(ln.type);

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

        // Displayhöhe = 48
        minOffsetY = min(0, 48 - y);
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
                u8g2.drawStr(0, y + fh, ln.text.c_str());

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

    // ---------- Scroll ----------
    int getStep()
    {
        for (auto &l : rendered)
        {
            if (l.type != 3)
                return min(4, max<int>(1, getFH(l.type) / 3));
        }
        return 3;
    }

    int update()
    {
        static bool lastBtn = HIGH;
        bool btn = digitalRead(PIN_BUTTON);

        if (btn == LOW && lastBtn == HIGH)
        {
            pressStart = millis();
            isHolding = false;
        }

        if (btn == LOW && (millis() - pressStart > HOLD_THRESHOLD))
            isHolding = true;

        if (btn == HIGH && lastBtn == LOW)
        {
            if (!isHolding)
            {
                if (mode == DOWN)
                    mode = STOP1;
                else if (mode == STOP1)
                    mode = UP;
                else if (mode == UP)
                    mode = STOP2;
                else
                    mode = DOWN;
            }
        }

        if (btn == HIGH && lastBtn == LOW && isHolding)
            isHolding = false;

        lastBtn = btn;

        int step = getStep();
        if (isHolding)
            step *= 3;

        if (millis() - lastScroll > 80)
        {
            lastScroll = millis();

            if (mode == DOWN)
                offsetY += step;
            else if (mode == UP)
                offsetY -= step;

            // CLAMP
            if (offsetY > 0)
            {
                offsetY = 0;
                mode = STOP1;
            }
            if (offsetY < minOffsetY)
            {
                offsetY = minOffsetY;
                mode = STOP2;
            }

            draw();
        }

        return 0;
    }
};

int renderArticle(Article content)
{
    ArticleRenderer r(content);
    r.draw();
    while (true)
    {
        int res = r.update();
        if (res != 0)
            return res;
    }
}