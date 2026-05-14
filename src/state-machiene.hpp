#pragma once

#include <Arduino.h>
#include <vector>
#include <string>
#include <climits>

#include <esp_sleep.h>
#include <driver/gpio.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

#include "config.hpp"
#include "display.hpp"
#include "article-renderer.hpp"
#include "data.hpp"

bool isMenuOpen = true;
std::vector<std::string> currentPath;
uint8_t menuState = 0;

// ─── Button (menu / edit / shutdown) ─────────────────────────────────────────

enum ButtonEvent
{
    BTN_NONE,
    BTN_CLICK,
    BTN_HOLD,
    BTN_DOUBLE_CLICK
};

ButtonEvent readButton()
{
    if (digitalRead(PIN_BUTTON) != LOW)
        return BTN_NONE;

    uint32_t pressedAt = millis();
    while (digitalRead(PIN_BUTTON) == LOW)
    {
        if (millis() - pressedAt > 500)
        {
            while (digitalRead(PIN_BUTTON) == LOW)
                ;
            delay(50);
            return BTN_HOLD;
        }
    }
    delay(50);

    uint32_t releasedAt = millis();
    while (millis() - releasedAt < 250)
    {
        if (digitalRead(PIN_BUTTON) == LOW)
        {
            while (digitalRead(PIN_BUTTON) == LOW)
                ;
            delay(50);
            return BTN_DOUBLE_CLICK;
        }
    }
    return BTN_CLICK;
}

// ─── Splash ───────────────────────────────────────────────────────────────────

void showSplash()
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_10x20_tr);
    u8g2.drawStr(16, 30, "W2GO");
    u8g2.sendBuffer();
    delay(1000);

    u8g2.clearBuffer();
    u8g2.drawBox(0, 0, 72, 40);
    u8g2.sendBuffer();
    delay(80);

    u8g2.clearBuffer();
    u8g2.sendBuffer();
    delay(80);
}

// ─── Menu ─────────────────────────────────────────────────────────────────────

void menu()
{
    static const std::vector<std::string> menuItems = {"Artikel", "Edit", "Aus"};

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_8x13_tr);
    for (size_t i = 0; i < menuItems.size(); i++)
    {
        if (i == menuState)
        {
            u8g2.drawBox(0, i * 13, 72, 13);
            u8g2.setDrawColor(0);
            u8g2.drawStr(2, (i + 1) * 13 - 2, menuItems[i].c_str());
            u8g2.setDrawColor(1);
        }
        else
        {
            u8g2.drawStr(2, (i + 1) * 13 - 2, menuItems[i].c_str());
        }
    }
    u8g2.sendBuffer();

    switch (readButton())
    {
    case BTN_CLICK:
        menuState = (menuState + 1) % menuItems.size();
        break;
    case BTN_HOLD:
        isMenuOpen = false;
        break;
    default:
        break;
    }
    delay(20);
}

// ─── List Browser ─────────────────────────────────────────────────────────────

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
    size_t selected = SIZE_MAX;

    int offsetY = 0;
    int minOffsetY = 0;
    unsigned long lastScroll = 0;

    enum ScrollMode
    {
        DOWN,
        STOP1,
        UP,
        STOP2
    } mode = STOP1;

    unsigned long pressStart = 0;
    bool isHolding = false;
    bool lastBtn = false;
    unsigned long lastRelease = 0;
    bool waitingDblClick = false;

    static constexpr int LINE_H = 13;
    static constexpr int ITEM_GAP = 6;
    static constexpr int DISPLAY_H = 40;
    static constexpr int MAX_CHARS = 12;
    static constexpr unsigned long HOLD_MS = 500;

    // ── Build ─────────────────────────────────────────────────────────────

    std::vector<std::string> wrapText(const std::string &txt)
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

    void build()
    {
        lines.clear();
        for (size_t i = 0; i < items.size(); i++)
        {
            auto wrapped = wrapText(items[i]);
            for (size_t j = 0; j < wrapped.size(); j++)
                lines.push_back({wrapped[j], i, j == 0});
        }
    }

    void calcHeight()
    {
        int y = 0;
        size_t prev = SIZE_MAX;
        for (auto &l : lines)
        {
            if (l.itemIndex != prev && prev != SIZE_MAX)
                y += ITEM_GAP;
            y += LINE_H;
            prev = l.itemIndex;
        }
        minOffsetY = std::min(0, DISPLAY_H - y - 4);
    }

    void reset(const std::vector<std::string> &it)
    {
        items = it;
        selected = (items.empty() ? SIZE_MAX : 0);

        offsetY = 0;
        mode = STOP1;

        isHolding = false;
        waitingDblClick = false;
        pressStart = 0;
        lastRelease = 0;

        lastBtn = false;

        build();
        calcHeight();
        syncSelected();
        draw();
    }

    // Clears button state after returning from a sub-screen (article, etc.)
    void resetButtonState()
    {
        lastBtn = (digitalRead(PIN_BUTTON) == LOW);
        isHolding = false;
        waitingDblClick = false;
        pressStart = 0;
        lastRelease = 0;
    }

    // ── Selected tracking ─────────────────────────────────────────────────

    void syncSelected()
    {
        int target = -offsetY;
        int bestDist = INT_MAX;
        int y = 0;
        size_t prev = SIZE_MAX;

        bool found = false;

        for (auto &l : lines)
        {
            if (l.itemIndex != prev && prev != SIZE_MAX)
                y += ITEM_GAP;

            if (l.firstLine)
            {
                int d = abs(y - target);
                if (d < bestDist)
                {
                    bestDist = d;
                    selected = l.itemIndex;
                    found = true;
                }
            }

            y += LINE_H;
            prev = l.itemIndex;
        }

        if (!found)
            selected = (items.empty() ? SIZE_MAX : 0);
    }

    // ── Draw ─────────────────────────────────────────────────────────────

    void draw()
    {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tr);

        int y = offsetY;
        size_t prev = SIZE_MAX;

        for (auto &l : lines)
        {
            if (l.itemIndex != prev && prev != SIZE_MAX)
                y += ITEM_GAP;

            if (y + LINE_H > 0 && y < DISPLAY_H)
            {
                int baseline = y + LINE_H - 2;
                if (l.firstLine && l.itemIndex == selected)
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

            y += LINE_H;
            prev = l.itemIndex;
        }

        u8g2.sendBuffer();
    }

    // ── Update ───────────────────────────────────────────────────────────
    //  Returns   0  → browsing
    //           -1  → double-click → back
    //           ≥0  → long-press   → selected item index

    int update()
    {
        bool btn = (digitalRead(PIN_BUTTON) == LOW);
        unsigned long now = millis();

        if (btn && !lastBtn)
        {
            pressStart = now;
            isHolding = false;
        }

        if (btn && lastBtn && !isHolding && (now - pressStart > HOLD_MS))
        {
            isHolding = true;

            while (digitalRead(PIN_BUTTON) == LOW)
                ;
            delay(50);

            lastBtn = false;
            return (int)selected;
        }

        if (!btn && lastBtn && !isHolding)
        {
            if (waitingDblClick && (now - lastRelease <= 300))
            {
                waitingDblClick = false;
                lastBtn = false;
                return -1;
            }

            waitingDblClick = true;
            lastRelease = now;

            switch (mode)
            {
            case DOWN:
                mode = STOP1;
                break;
            case STOP1:
                mode = UP;
                break;
            case UP:
                mode = STOP2;
                break;
            case STOP2:
                mode = DOWN;
                break;
            }

            isHolding = false;
        }

        if (waitingDblClick && (now - lastRelease > 300))
            waitingDblClick = false;

        lastBtn = btn;

        if (now - lastScroll > 80)
        {
            lastScroll = now;

            if (mode == DOWN)
                offsetY += 2;
            else if (mode == UP)
                offsetY -= 2;

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

            syncSelected();
            draw();
        }

        return 0;
    }
};

// ─── Article browser ─────────────────────────────────────────────────────────

void renderArticleUI()
{
    static ListBrowser lb;
    static std::string lastDir = "";
    static bool ready = false;

    std::string dir = "/";
    for (const auto &seg : currentPath)
        dir += seg + "/";

    if (!ready || dir != lastDir)
    {
        std::vector<std::string> entries = readDir(dir.c_str());

        if (entries.empty())
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

        lb.reset(entries);
        lastDir = dir;
        ready = true;
        return;
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
    else if (res >= 0 && (size_t)res < lb.items.size())
    {
        const std::string &entry = lb.items[(size_t)res];

        if (entry.size() > 4 && entry.compare(entry.size() - 4, 4, ".bin") == 0)
        {
            renderArticle(readArticleBinary((dir + entry).c_str()));
            // Clear stale button state accumulated inside the article renderer
            lb.resetButtonState();
            lb.draw();
        }
        else
        {
            currentPath.push_back(entry);
            ready = false;
        }
    }
}

// ─── Edit / Wi-Fi portal ──────────────────────────────────────────────────────

static WebServer httpServer(80);
static DNSServer dnsServer;
static bool wifiRunning = false;

static const char PORTAL_HTML[] PROGMEM = R"(<!DOCTYPE html>
<html><head><meta charset='utf-8'>
<meta name='viewport' content='width=device-width,initial-scale=1'>
<title>W2GO</title>
<style>body{font-family:sans-serif;max-width:480px;margin:2rem auto;padding:1rem}
h1{font-size:1.4rem}button{width:100%;padding:.5rem;margin:.4rem 0;
background:#222;color:#fff;border:none;cursor:pointer}</style></head>
<body><h1>W2GO Dateien</h1>
<p>Verbunden. Lege .bin-Artikel auf das Gerät.</p>
<p><small>Beenden: langer Tastendruck am Gerät.</small></p>
</body></html>)";

static void startWifi()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP("W2GO", nullptr, 1, false, 1);
    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    dnsServer.start(53, "*", apIP);
    httpServer.onNotFound([]()
                          { httpServer.send_P(200, "text/html", PORTAL_HTML); });
    httpServer.begin();
    wifiRunning = true;
}

static void stopWifi()
{
    httpServer.stop();
    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    wifiRunning = false;
}

void renderEditUI()
{
    if (!wifiRunning)
        startWifi();

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(2, 10, "SSID:");
    u8g2.drawStr(2, 20, "W2GO");
    u8g2.drawStr(2, 32, "192.168.4.1");
    u8g2.drawStr(2, 40, "[2x=Beenden]");
    u8g2.sendBuffer();

    dnsServer.processNextRequest();
    httpServer.handleClient();

    if (readButton() == BTN_DOUBLE_CLICK)
    {
        stopWifi();
        isMenuOpen = true;
        menuState = 0;
    }
    delay(10);
}

// ─── Shutdown ─────────────────────────────────────────────────────────────────

void renderShutdownUI()
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_8x13_tr);
    u8g2.drawStr(4, 15, "Schlafen?");
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(2, 28, "Hold=Ja");
    u8g2.drawStr(2, 38, "2x=Zuruck");
    u8g2.sendBuffer();

    switch (readButton())
    {
    case BTN_HOLD:
    {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_8x13_tr);
        u8g2.drawStr(20, 24, "Tschuss!");
        u8g2.sendBuffer();
        delay(800);

        u8g2.clearBuffer();
        u8g2.sendBuffer();
        u8g2.setPowerSave(1);

        if (wifiRunning)
            stopWifi();

        // Configure PIN_BUTTON (boot/GPIO9) as input with pull-up via IDF
        // driver — Arduino's pinMode does not persist into deep sleep on C3.
        gpio_reset_pin((gpio_num_t)PIN_BUTTON);
        gpio_set_direction((gpio_num_t)PIN_BUTTON, GPIO_MODE_INPUT);
        gpio_pullup_en((gpio_num_t)PIN_BUTTON);

        esp_deep_sleep_enable_gpio_wakeup(1ULL << PIN_BUTTON,
                                          ESP_GPIO_WAKEUP_GPIO_LOW);
        esp_deep_sleep_start();
        break; // unreachable
    }
    case BTN_DOUBLE_CLICK:
        isMenuOpen = true;
        menuState = 0;
        break;
    default:
        break;
    }
    delay(20);
}

// ─── Top-level loop ───────────────────────────────────────────────────────────

void loopUI()
{
    if (isMenuOpen)
    {
        menu();
        return;
    }

    switch (menuState)
    {
    case 0:
        renderArticleUI();
        break;
    case 1:
        renderEditUI();
        break;
    case 2:
        renderShutdownUI();
        break;
    }
}

void stateMachine()
{
    showSplash();
    while (true)
        loopUI();
}