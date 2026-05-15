#pragma once

#include <Arduino.h>
#include <string>
#include <vector>

struct ListBrowser
{
    struct RenderLine
    {
        std::string text;
        size_t itemIndex;
        bool firstLine;
    };

    std::vector<std::string> items;
    std::vector<RenderLine> lines;

    int selected = 0;
    int scrollOffset = 0;

    static constexpr int LINE_H = 12;
    static constexpr int DISPLAY_H = 40;
    static constexpr int MAX_LINES = 3;
    static constexpr int MAX_CHARS = 12;

    std::vector<std::string> wrapText(const std::string &txt);
    void build();
    void reset(const std::vector<std::string> &it);
    void draw();
    int update();
};
