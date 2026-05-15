#include "list_browser.hpp"
#include "button.hpp"
#include "../display.hpp"

std::vector<std::string> ListBrowser::wrapText(const std::string &txt)
{
    std::vector<std::string> out;
    std::string s = txt;

    while (s.size() > (size_t)MAX_CHARS)
    {
        size_t cut = s.rfind(' ', MAX_CHARS);
        if (cut != std::string::npos && cut > 0)
        {
            out.push_back(s.substr(0, cut));
            s = s.substr(cut + 1);
        }
        else
        {
            out.push_back(s.substr(0, MAX_CHARS - 1) + "-");
            s = s.substr(MAX_CHARS - 1);
        }
    }

    if (!s.empty())
        out.push_back(s);

    return out;
}

void ListBrowser::build()
{
    lines.clear();
    for (size_t i = 0; i < items.size(); i++)
    {
        auto wrapped = wrapText(items[i]);
        for (size_t j = 0; j < wrapped.size(); j++)
            lines.push_back({wrapped[j], i, j == 0});
    }
}

void ListBrowser::reset(const std::vector<std::string> &it)
{
    items = it;
    selected = 0;
    scrollOffset = 0;
    build();
    draw();
}

void ListBrowser::draw()
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);

    int selectedLineIdx = 0;
    for (int i = 0; i < (int)lines.size(); i++)
    {
        if (lines[i].itemIndex == (size_t)selected && lines[i].firstLine)
        {
            selectedLineIdx = i;
            break;
        }
    }

    if (selectedLineIdx < scrollOffset)
        scrollOffset = selectedLineIdx;
    else if (selectedLineIdx >= scrollOffset + MAX_LINES)
        scrollOffset = selectedLineIdx - MAX_LINES + 1;

    for (int i = 0; i < MAX_LINES && (scrollOffset + i) < (int)lines.size(); i++)
    {
        auto &l = lines[scrollOffset + i];
        int y = i * LINE_H;
        int baseline = y + LINE_H - 2;

        if (l.itemIndex == (size_t)selected && l.firstLine)
        {
            u8g2.drawBox(0, y, 72, LINE_H);
            u8g2.setDrawColor(0);
            u8g2.drawStr(2, baseline, l.text.c_str());
            u8g2.setDrawColor(1);
        }
        else
        {
            u8g2.drawStr(l.firstLine ? 2 : 8, baseline, l.text.c_str());
        }
    }

    u8g2.sendBuffer();
}

int ListBrowser::update()
{
    ButtonEvent ev = readButton();
    if (ev == BTN_CLICK)
    {
        selected = (selected + 1) % items.size();
        draw();
    }
    else if (ev == BTN_HOLD)
    {
        return selected;
    }
    else if (ev == BTN_DOUBLE_CLICK)
    {
        return -1;
    }
    return -2;
}
