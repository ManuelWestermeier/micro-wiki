#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include <vector>

enum Type
{
    H,
    T,
    LI
};

struct Item
{
    Type typ;
    std::vector<String> daten;
};

struct Line
{
    String text;
    uint8_t type;
};

using Article = std::vector<Item>;

class ArticleRenderer
{
public:
    explicit ArticleRenderer(const Article &content);
    void draw();
    int update();

private:
    Article content;
    std::vector<Line> rendered;
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
    const unsigned long LONG_PRESS_THRESHOLD = 1500;

    bool lastBtn = HIGH;
    unsigned long lastRelease = 0;
    bool waitingSecondClick = false;
    bool pendingSingleClick = false;

    void setFont(uint8_t t);
    int getFH(uint8_t t) const;
    int getMaxChars(uint8_t t) const;
    std::vector<String> wrap(const String &txt, int maxC) const;
    void build();
    void calculateHeight();
    int getStep() const;
};

int renderArticle(const Article &content);
