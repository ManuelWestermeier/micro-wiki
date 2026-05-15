#include "wifi_portal.hpp"
#include "button.hpp"
#include "../display.hpp"
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

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
    u8g2.drawStr(2, 40, "[Hold=Beenden]");
    u8g2.sendBuffer();

    dnsServer.processNextRequest();
    httpServer.handleClient();

    if (readButton() == BTN_DOUBLE_CLICK)
    {
        stopWifi();
        extern bool isMenuOpen;
        extern uint8_t menuState;
        isMenuOpen = true;
        menuState = 0;
    }

    delay(10);
}
