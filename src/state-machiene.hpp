#pragma once

#include <Arduino.h>
#include <vector>
#include <string>

#include <esp_sleep.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

#include "config.hpp"
#include "display.hpp"
#include "article-renderer.hpp"
#include "data.hpp"

// NOTE: defining variables in a header causes ODR violations if included in
// multiple translation units. Move definitions to a ui.cpp and keep only
// `extern` declarations here.
bool isMenuOpen = true;
std::vector<std::string> currentPath;
uint8_t menuState = 0;

// ─── Button ──────────────────────────────────────────────────────────────────

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

    // Wait for release or hold threshold
    while (digitalRead(PIN_BUTTON) == LOW)
    {
        if (millis() - pressedAt > 500)
        {
            while (digitalRead(PIN_BUTTON) == LOW)
                ; // wait for release
            delay(50);
            return BTN_HOLD;
        }
    }
    delay(50); // debounce

    // Listen for a second press within 250 ms → double click
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

    // flash white → black
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
        isMenuOpen = false; // confirm selection → enter sub-UI
        break;
    default:
        break;
    }

    delay(20);
}

// ─── Article browser ─────────────────────────────────────────────────────────
// Navigation: click = next item, hold = open, double-click = back
// currentPath tracks the directory stack; an empty path means root.

static void drawList(const std::vector<std::string> &items, size_t selected)
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);

    // Show up to 4 rows; scroll window follows the selection
    const size_t visibleRows = 4;
    size_t offset = (selected >= visibleRows) ? selected - visibleRows + 1 : 0;

    for (size_t i = 0; i < visibleRows && (offset + i) < items.size(); i++)
    {
        size_t idx = offset + i;
        int y = (i + 1) * 10 - 1;

        if (idx == selected)
        {
            u8g2.drawBox(0, i * 10, 72, 10);
            u8g2.setDrawColor(0);
            u8g2.drawStr(2, y, items[idx].c_str());
            u8g2.setDrawColor(1);
        }
        else
        {
            u8g2.drawStr(2, y, items[idx].c_str());
        }
    }
    u8g2.sendBuffer();
}

void renderArticleUI()
{
    static size_t selectedIndex = 0;

    // Build the path string for readDir
    std::string dir = "/";
    for (const auto &seg : currentPath)
        dir += seg + "/";

    std::vector<std::string> entries = readDir(dir.c_str());

    if (entries.empty())
    {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tr);
        u8g2.drawStr(2, 20, "Leer.");
        u8g2.sendBuffer();
        delay(800);

        // go back automatically
        if (!currentPath.empty())
            currentPath.pop_back();
        else
        {
            isMenuOpen = true;
            menuState = 0;
        }
        selectedIndex = 0;
        return;
    }

    // clamp selection after directory changes
    if (selectedIndex >= entries.size())
        selectedIndex = 0;

    drawList(entries, selectedIndex);

    switch (readButton())
    {
    case BTN_CLICK:
        selectedIndex = (selectedIndex + 1) % entries.size();
        break;

    case BTN_HOLD:
    {
        const std::string &entry = entries[selectedIndex];

        // .bin files are articles; everything else is a directory
        if (entry.size() > 4 &&
            entry.compare(entry.size() - 4, 4, ".bin") == 0)
        {
            std::string articlePath = dir + entry;
            renderArticle(readArticleBinary(articlePath.c_str()));
            // renderArticle blocks until the user exits it
        }
        else
        {
            currentPath.push_back(entry);
            selectedIndex = 0;
        }
        break;
    }

    case BTN_DOUBLE_CLICK:
        if (!currentPath.empty())
        {
            currentPath.pop_back();
            selectedIndex = 0;
        }
        else
        {
            // back to main menu from root
            isMenuOpen = true;
            menuState = 0;
            selectedIndex = 0;
        }
        break;

    default:
        break;
    }

    delay(20);
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
h1{font-size:1.4rem}input,button{width:100%;padding:.5rem;margin:.4rem 0;box-sizing:border-box}
button{background:#222;color:#fff;border:none;cursor:pointer}</style></head>
<body><h1>W2GO Dateien</h1>
<p>Verbunden. Lege .bin-Artikel unter /artikel/ ab.</p>
<p><small>Zum Beenden: langer Tastendruck am Gerät.</small></p>
</body></html>)";

static void startWifi()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP("W2GO", nullptr, 1, false, 1); // open AP, channel 1, max 1 client

    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

    // Captive-portal DNS: redirect everything to our IP
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

    // Display SSID + IP
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(2, 10, "SSID:");
    u8g2.drawStr(2, 20, "W2GO");
    u8g2.drawStr(2, 32, "192.168.4.1");
    u8g2.drawStr(2, 40, "[Hold=Beenden]");
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

// ─── Shutdown / deep sleep ────────────────────────────────────────────────────

void renderShutdownUI()
{
    // Two-step confirm: first show prompt, hold to confirm, double-click to cancel
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
        // Brief "Bye" screen before sleeping
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

        esp_deep_sleep_enable_gpio_wakeup(1ULL << PIN_BUTTON, ESP_GPIO_WAKEUP_GPIO_LOW);
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
    {
        loopUI();
    }
}