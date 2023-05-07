#include "SPIFFS.h"
#include <WebServer.h>
#include "ESPAsyncWebServer.h"
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include "lib/functions.hpp"
#include <string>

class OneParamRewrite : public AsyncWebRewrite
{
protected:
    String _urlPrefix;
    int _paramIndex;
    String _paramsBackup;

public:
    OneParamRewrite(const char *from, const char *to)
        : AsyncWebRewrite(from, to)
    {

        _paramIndex = _from.indexOf('{');

        if (_paramIndex >= 0 && _from.endsWith("}"))
        {
            _urlPrefix = _from.substring(0, _paramIndex);
            int index = _params.indexOf('{');
            if (index >= 0)
            {
                _params = _params.substring(0, index);
            }
        }
        else
        {
            _urlPrefix = _from;
        }
        _paramsBackup = _params;
    }

    bool match(AsyncWebServerRequest *request) override
    {
        if (request->url().startsWith(_urlPrefix))
        {
            if (_paramIndex >= 0)
            {
                _params = _paramsBackup + request->url().substring(_paramIndex);
            }
            else
            {
                _params = _paramsBackup;
            }
            return true;
        }
        else
        {
            return false;
        }
    }
};

AsyncWebServer server(80);

void setupWebserver()
{
    // Initialize SPIFFS
    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html", String(), false); });

    server.addRewrite(new OneParamRewrite("/api/show/screen/{s}", "/api/show/screen?s={s}"));
    server.addRewrite(new OneParamRewrite("/api/show/text/{text}", "/api/show/text?t={text}"));
    server.addRewrite(new OneParamRewrite("/api/show/number/{number}", "/api/show/number?t={number}"));

    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        DynamicJsonDocument root(1024);
        root["currentScreen"] = String(currentScreen);
        root["rendered"] = blocks;
        JsonArray data = root.createNestedArray("data");
        copyArray(epdContent, data);
        String responseText;
        serializeJson(root, responseText);

        request->send(200, "application/json", responseText); });

    server.on("/api/action/pause", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        timerRunning = false;
        Serial.println("Update timer paused");

        request->send(200); });

    server.on("/api/full_refresh", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        fullRefresh();

        request->send(200); });

    server.on("/api/action/timer_restart", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        moment = millis();
        timerRunning = true;
        Serial.println("Update timer restarted");

        request->send(200); });

    server.on("/api/action/update", HTTP_GET, [](AsyncWebServerRequest *request)
              {

        if(request->hasParam("rate")) {
            AsyncWebParameter* p = request->getParam("rate");
            timerSeconds = stoi(p->value().c_str()) * 60;
            preferences.putUInt("timerSeconds", timerSeconds);
        } 
                        
        moment = millis();
        timerRunning = true;

        request->send(200); });

    server.on("/api/settings", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        DynamicJsonDocument root(1024);
        root["fgColor"] = FG_COLOR;
        root["bgColor"] = BG_COLOR;
        root["timerSeconds"] = timerSeconds;
        root["timerRunning"] = timerRunning;
#ifdef IS_BW
        root["epdColors"] = 2;
#else
        root["epdColors"] = 3;
#endif
        String responseText;
        serializeJson(root, responseText);
        request->send(200, "application/json", responseText); });

    server.on("/api/settings", HTTP_POST, [](AsyncWebServerRequest *request)
              {
        int params = request->params();
        bool settingsChanged = false;
        if(request->hasParam("fgColor", true)) {
            AsyncWebParameter* fgColor = request->getParam("fgColor", true);
            preferences.putUInt("fgColor", strtol(fgColor->value().c_str(), NULL, 16));
            Serial.print("Setting foreground color to ");
            Serial.println(fgColor->value().c_str());
            settingsChanged = true;
        }
        if(request->hasParam("bgColor", true)) {
            AsyncWebParameter* bgColor = request->getParam("bgColor", true);

            preferences.putUInt("bgColor", strtol(bgColor->value().c_str(), NULL, 16));
            Serial.print("Setting background color to ");
            Serial.println(bgColor->value().c_str());
            settingsChanged = true;
        }
        
        request->send(200);
        if (settingsChanged) {
            Serial.println("Settings changed");
        } });

    server.on("/api/show/screen", HTTP_GET, [](AsyncWebServerRequest *request)
              { 
                //Check if GET parameter exists
                if(request->hasParam("s")) {
                    AsyncWebParameter* p = request->getParam("s");
                    currentScreen = stoi(p->value().c_str());
                } 
                request->send(200); });

    server.on("/api/show/text", HTTP_GET, [](AsyncWebServerRequest *request)
              { 
                //Check if GET parameter exists
                if(request->hasParam("t")) {
                    AsyncWebParameter* p = request->getParam("t");
                    showText(p->value().c_str());
                } 
                request->send(200); });

    server.on("/api/show/number", HTTP_GET, [](AsyncWebServerRequest *request)
              { 
                //Check if GET parameter exists
                if(request->hasParam("pair")) {
                    AsyncWebParameter* pair = request->getParam("pair");
                    AsyncWebParameter* t = request->getParam("t");
                    String top = (pair->value().substring(0, pair->value().indexOf("/")));
                    String bottom = pair->value().substring((pair->value().indexOf("/")+1)); 

                    showNumberWithPair(top.c_str(), bottom.c_str(), t->value().c_str());
                } 
                request->send(200); });

    server.onNotFound([](AsyncWebServerRequest *request)
                      {
    if (request->method() == HTTP_OPTIONS) {
        request->send(200);
    } else {
        request->send(404);
    } });

    server.serveStatic("/css", SPIFFS, "/css/");
    server.serveStatic("/js", SPIFFS, "/js/");
    server.serveStatic("/font", SPIFFS, "/font/");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

    // Start server
    server.begin();
    if (!MDNS.begin(OTA_NAME))
    {
        Serial.println("Error setting up MDNS responder!");
        while (1)
        {
            delay(1000);
        }
    }
    MDNS.addService("http", "tcp", 80);
}