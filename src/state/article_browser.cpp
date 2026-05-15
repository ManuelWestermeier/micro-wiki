#include "article_browser.hpp"
#include "list_browser.hpp"
#include "../data.hpp"
#include "../display.hpp"
#include "../article-renderer.hpp"
#include <vector>
#include <string>

extern bool isMenuOpen;
extern std::vector<std::string> currentPath;
extern uint8_t menuState;

static std::string stripBinSuffix(const std::string &name)
{
    if (name.size() > 4 && name.substr(name.size() - 4) == ".bin")
        return name.substr(0, name.size() - 4);
    return name;
}

void renderArticleUI()
{
    static ListBrowser lb;
    static std::string lastDir;
    static bool ready = false;

    std::string dir = "/";
    for (const auto &seg : currentPath)
        dir += seg + "/";

    if (!ready || dir != lastDir)
    {
        std::vector<std::string> rawEntries = readDir(dir);
        if (rawEntries.empty())
        {
            u8g2.clearBuffer();
            u8g2.setFont(u8g2_font_6x10_tr);
            u8g2.drawStr(2, 20, "Leer.");
            u8g2.sendBuffer();
            delay(800);

            if (!currentPath.empty())
                currentPath.pop_back();
            else
            {
                isMenuOpen = true;
                menuState = 0;
            }

            ready = false;
            return;
        }

        std::vector<std::string> displayEntries;
        displayEntries.reserve(rawEntries.size());
        for (const auto &entry : rawEntries)
            displayEntries.push_back(stripBinSuffix(entry));

        lb.reset(displayEntries);
        lastDir = dir;
        ready = true;
    }

    int res = lb.update();

    if (res == -1)
    {
        ready = false;
        if (!currentPath.empty())
            currentPath.pop_back();
        else
        {
            isMenuOpen = true;
            menuState = 0;
        }
    }
    else if (res >= 0)
    {
        std::vector<std::string> rawEntries = readDir(dir);
        const std::string &entry = rawEntries[res];
        if (entry.size() > 4 && entry.substr(entry.size() - 4) == ".bin")
        {
            renderArticle(readArticleBinary(dir + entry));
            lb.draw();
        }
        else
        {
            currentPath.push_back(entry);
            ready = false;
        }
    }
}
