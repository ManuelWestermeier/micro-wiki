#include "article_renderer.hpp"
#include "../display.hpp"
#include "../config.hpp"

ArticleRenderer::ArticleRenderer(const Article &content)
    : content(content)
{
    build();
    calculateHeight();
    lastBtn = digitalRead(PIN_BUTTON);
}

void ArticleRenderer::setFont(uint8_t t)
{
    if (t == 0)
        u8g2.setFont(u8g2_font_7x13B_tf);
    else if (t == 1)
        u8g2.setFont(u8g2_font_6x12_tf);
    else
        u8g2.setFont(u8g2_font_6x10_tf);
}

int ArticleRenderer::getFH(uint8_t t) const
{
    if (t == 0)
        return 14;
    if (t == 1)
        return 12;
    return 10;
}

int ArticleRenderer::getMaxChars(uint8_t t) const
{
    if (t == 0)
        return 9;
    if (t == 2)
        return 11;
    return 12;
}

std::vector<String> ArticleRenderer::wrap(const String &txt, int maxC) const
{
    std::vector<String> out;
    String remaining = txt;

    while (remaining.length())
    {
        if (remaining.length() <= maxC)
        {
            out.push_back(remaining);
            break;
        }

        int cut = remaining.lastIndexOf(' ', maxC);
        if (cut > 0)
        {
            out.push_back(remaining.substring(0, cut));
            remaining = remaining.substring(cut + 1);
        }
        else
        {
            out.push_back(remaining.substring(0, maxC - 1) + "-");
            remaining = remaining.substring(maxC - 1);
        }
    }
    return out;
}

void ArticleRenderer::build()
{
    rendered.clear();

    for (auto &item : content)
    {
        if (item.typ == H)
        {
            auto lines = wrap(item.daten[0], getMaxChars(0));
            for (auto &line : lines)
                rendered.push_back({line, 0});
        }
        else if (item.typ == T)
        {
            auto lines = wrap(item.daten[0], getMaxChars(1));
            for (auto &line : lines)
                rendered.push_back({line, 1});
        }
        else if (item.typ == LI)
        {
            for (auto &entry : item.daten)
            {
                auto lines = wrap(entry, getMaxChars(2));
                if (!lines.empty())
                    lines[0] = "> " + lines[0];
                for (auto &line : lines)
                    rendered.push_back({line, 2});
            }
        }
        rendered.push_back({"", 3});
    }

    rendered.push_back({"-- ENDE --", 0});
    rendered.push_back({"", 3});
    rendered.push_back({"", 3});
}

void ArticleRenderer::calculateHeight()
{
    int y = 0;

    for (size_t i = 0; i < rendered.size(); i++)
    {
        auto &ln = rendered[i];
        if (ln.type == 3)
        {
            y += 14;
            continue;
        }

        int fh = getFH(ln.type);
        int spacing = 2;
        if (ln.type == 0)
            spacing = 5;
        if (ln.type == 2)
            spacing = ((i + 1 < rendered.size() && rendered[i + 1].type == 2) ? 8 : 2);

        y += fh + spacing;
    }

    minOffsetY = min(0, 48 - y);
}

void ArticleRenderer::draw()
{
    u8g2.clearBuffer();
    int y = offsetY;

    for (size_t i = 0; i < rendered.size(); i++)
    {
        auto &ln = rendered[i];

        if (ln.type == 3)
        {
            y += 14;
            continue;
        }

        setFont(ln.type);
        int fh = getFH(ln.type);
        int x = (ln.type == 2) ? 4 : 0;

        if (y > -fh && y < 48)
            u8g2.drawStr(x, y + fh, ln.text.c_str());

        int spacing = 2;
        if (ln.type == 0)
            spacing = 5;
        if (ln.type == 2)
            spacing = ((i + 1 < rendered.size() && rendered[i + 1].type == 2) ? 8 : 2);

        y += fh + spacing;
    }

    u8g2.sendBuffer();
}

int ArticleRenderer::getStep() const
{
    for (auto &ln : rendered)
    {
        if (ln.type != 3)
            return min(4, max<int>(1, getFH(ln.type) / 3));
    }
    return 3;
}

int ArticleRenderer::update()
{
    bool btn = digitalRead(PIN_BUTTON);
    unsigned long now = millis();

    if (btn == LOW && lastBtn == HIGH)
    {
        pressStart = now;
        isHolding = false;
    }

    if (btn == LOW && lastBtn == LOW)
    {
        if (now - pressStart > LONG_PRESS_THRESHOLD && (mode == STOP1 || mode == STOP2))
        {
            while (digitalRead(PIN_BUTTON) == LOW)
                delay(10);
            lastBtn = HIGH;
            isHolding = false;
            waitingSecondClick = false;
            pendingSingleClick = false;
            return 1;
        }
        else if (now - pressStart > HOLD_THRESHOLD)
        {
            isHolding = true;
        }
    }

    if (btn == HIGH && lastBtn == LOW)
    {
        unsigned long duration = now - pressStart;
        if (duration < HOLD_THRESHOLD)
        {
            if (waitingSecondClick && (now - lastRelease <= 300))
            {
                waitingSecondClick = false;
                pendingSingleClick = false;
                lastBtn = HIGH;
                return 1;
            }
            else
            {
                waitingSecondClick = true;
                lastRelease = now;
                pendingSingleClick = true;
            }
        }
        isHolding = false;
    }

    if (pendingSingleClick && (now - lastRelease > 300))
    {
        pendingSingleClick = false;
        if (mode == DOWN)
            mode = STOP1;
        else if (mode == STOP1)
            mode = UP;
        else if (mode == UP)
            mode = STOP2;
        else
            mode = DOWN;
    }

    if (waitingSecondClick && (now - lastRelease > 300))
        waitingSecondClick = false;

    lastBtn = btn;

    int step = getStep();
    if (isHolding)
        step *= 3;

    if (now - lastScroll > 80)
    {
        lastScroll = now;
        if (mode == DOWN)
            offsetY += step;
        else if (mode == UP)
            offsetY -= step;

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

int renderArticle(const Article &content)
{
    ArticleRenderer renderer(content);
    renderer.draw();
    while (true)
    {
        int res = renderer.update();
        if (res != 0)
            return res;
    }
}
