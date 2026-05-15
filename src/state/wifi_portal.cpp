#include "wifi_portal.hpp"
#include "button.hpp"
#include "../display.hpp"
#include "../data.hpp"
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <FS.h>
#include <LittleFS.h>

static WebServer httpServer(80);
static DNSServer dnsServer;
static bool wifiRunning = false;

static void startWifi()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP("W2GO", nullptr, 1, false, 1);

    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

    dnsServer.start(53, "*", apIP);

    // Root page with file list and upload
    httpServer.on("/", HTTP_GET, []()
    {
        String html = R"(<!DOCTYPE html>
<html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>
<title>W2GO</title>
<style>
body{font-family:sans-serif;max-width:600px;margin:2rem auto;padding:1rem;background:#f5f5f5}
h1{font-size:1.8rem;color:#333}
.section{background:#fff;padding:1.5rem;margin:1rem 0;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1)}
input[type=file]{padding:0.5rem;margin:0.5rem 0;width:100%;box-sizing:border-box}
button{width:100%;padding:0.75rem;margin:0.5rem 0;background:#007bff;color:#fff;border:none;border-radius:4px;cursor:pointer;font-size:1rem}
button:hover{background:#0056b3}
button.delete{background:#dc3545}
button.delete:hover{background:#c82333}
.file-item{display:flex;justify-content:space-between;align-items:center;padding:0.75rem;background:#f9f9f9;margin:0.5rem 0;border-radius:4px;border-left:4px solid #007bff}
.file-name{flex:1;font-weight:500;color:#333}
.btn-group{display:flex;gap:0.5rem}
.btn-group button{flex:1;margin:0;padding:0.5rem}
</style></head>
<body>
<h1>📁 W2GO Dateien</h1>
<div class='section'>
<h2>Dateiübersicht</h2>)";

        // List files
        std::vector<std::string> files = readDir("/");
        if (files.empty())
        {
            html += "<p>Keine Dateien vorhanden.</p>";
        }
        else
        {
            for (const auto &file : files)
            {
                html += "<div class='file-item'>";
                html += "<span class='file-name'>";
                html += file.c_str();
                html += "</span>";
                html += "<div class='btn-group'>";
                html += "<button class='delete' onclick=\"if(confirm('Datei löschen?')) location='/delete?file=";
                html += file.c_str();
                html += "'\">Löschen</button>";
                html += "</div>";
                html += "</div>";
            }
        }

        html += R"(</div>
<div class='section'>
<h2>Neue Datei hochladen</h2>
<form method='POST' action='/upload' enctype='multipart/form-data'>
<input type='file' name='file' accept='.bin' required>
<button type='submit'>Hochladen</button>
</form>
</div>
<div class='section' style='background:#e7f3ff;border-left:4px solid #0056b3'>
<small>💡 Hinweis: Lege .bin-Dateien hier ab und lösche alte Artikel. 
Das Gerät funktioniert am besten mit Dateien unter 100 KB.</small>
</div>
</body></html>)";

        httpServer.send(200, "text/html", html);
    });

    // Delete file
    httpServer.on("/delete", HTTP_GET, []()
    {
        String filename = httpServer.arg("file");
        if (!filename.isEmpty())
        {
            String filepath = "/" + filename;
            if (LittleFS.exists(filepath.c_str()))
            {
                LittleFS.remove(filepath.c_str());
                httpServer.sendHeader("Location", "/");
                httpServer.send(302, "text/plain", "");
            }
            else
            {
                httpServer.send(404, "text/html", "<h1>Datei nicht gefunden</h1><a href='/'>Zurück</a>");
            }
        }
        else
        {
            httpServer.sendHeader("Location", "/");
            httpServer.send(302, "text/plain", "");
        }
    });

    // Upload file
    httpServer.on("/upload", HTTP_POST, []()
    {
        httpServer.sendHeader("Location", "/");
        httpServer.send(302, "text/plain", "");
    }, 
    []()
    {
        HTTPUpload &upload = httpServer.upload();
        static File uploadFile;

        if (upload.status == UPLOAD_FILE_START)
        {
            String filename = upload.filename;
            if (!filename.startsWith("/"))
                filename = "/" + filename;

            uploadFile = LittleFS.open(filename.c_str(), "w");
        }
        else if (upload.status == UPLOAD_FILE_WRITE)
        {
            if (uploadFile)
                uploadFile.write(upload.buf, upload.currentSize);
        }
        else if (upload.status == UPLOAD_FILE_END)
        {
            if (uploadFile)
                uploadFile.close();
        }
    });

    httpServer.onNotFound([]()
    {
        httpServer.sendHeader("Location", "/");
        httpServer.send(302, "text/plain", "");
    });

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
