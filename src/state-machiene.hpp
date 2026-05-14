#pragma once

#include <Arduino.h>

#include <vector>
#include <string>

#include "config.hpp"
#include "display.hpp"
#include "article-renderer.hpp"
#include "data.hpp"

bool isMenuOpen = true;
vector<string> currentPath;
uint8_t menuState = 0;

// 72x40
void start()
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_10x20_tr);
    u8g2.drawStr(16, 30, "W2GO");
    u8g2.sendBuffer();
    delay(1000);

    // white
    u8g2.clearBuffer();
    u8g2.drawBox(0, 0, 72, 40);
    u8g2.sendBuffer();

    // black
    u8g2.clearBuffer();
    u8g2.sendBuffer();
}

void menu()
{
    // make the menu drawing better. button clicks better capture
    vector<string> menuItems = {"Artikel", "Edit", "Aus"};

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
    if (digitalRead(PIN_BUTTON) == LOW)
    {
        auto start = millis();
        while (digitalRead(PIN_BUTTON) == LOW)
        {
            if (millis() - start > 500)
            {
                delay(10);
                isMenuOpen = false;
                return;
            }
            menuState = (menuState + 1) % menuItems.size();
        }

        u8g2.sendBuffer();
    }
    delay(20);
}

// click = down, hold = open, dbclick = back
void renderArticleUI()
{
    vector<string> articles = readDir("/");
    // secet cathegory, subcathegory, articles
    // => bsp.
    // renderArticle(readArticleBinary("/deutsch/grammatik_und_sprache/sprachsystem.bin"));
    // renderArticle return = back to articles
    // root + dbclick = menu
}

void renderEditUI()
{
    // create wif, show ssid, create minimal web http server, dns portal, that opens itselv
    // hello world server
}

void renderShutdownUI()
{
    // deepsleep confirm ui, wakeup = PIN_BUTTON
    //  always tell dbclick = back,
}

void loopUI()
{
    if (isMenuOpen)
        menu();
    else
    {
        if (menuState == 0)
        {
            renderArticleUI();
        }
        else if (menuState == 1)
        {
            renderEditUI();
        }
        else if (menuState == 2)
        {
            renderShutdownUI();
        }
    }
}

void stateMachine()
{
    start();
    while (true)
    {
        loopUI();
    }
}