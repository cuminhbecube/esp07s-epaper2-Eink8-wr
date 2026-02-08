#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecure.h>
#include <time.h>

#include <GxEPD2_BW.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include "../driver_config.h" 
#include "../include/weather_icons.h" 

// --- CONFIG ---
const char* FIRMWARE_URL = "https://raw.githubusercontent.com/cuminhbecube/firmware-esp07s-epaper2-Eink8-wr-/main/firmware.bin";
const float LAT = 20.97;
const float LON = 105.85;

// Timer
unsigned long lastTimeUpdate = 0;
unsigned long lastWeatherUpdate = 0;
const unsigned long TIME_UPDATE_INTERVAL = 60000;       
const unsigned long WEATHER_UPDATE_INTERVAL = 1800000;  

// Display
#include "epd/GxEPD2_750.h"
GxEPD2_BW<GxEPD2_750, GxEPD2_750::HEIGHT / 4> display(GxEPD2_750(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY));

struct DailyForecast {
    time_t date;
    int code;
    float maxTemp;
    float minTemp;
};

struct WeatherData {
    DailyForecast daily[4]; 
    float currentTemp;
    int currentCode;
    bool valid;
};
WeatherData weather = {{{0}}, 0, 0, false};

// Button Logic
const int BTN_PIN = PIN_SWITCH_1; 
int btnState;
int lastBtnState = HIGH;
unsigned long pressStartTime = 0;
bool isPressing = false;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;
const int   daylightOffset_sec = 0;

// Prototypes
void drawScreen();
void fetchWeather();
void performOTA();
void enterConfigMode();
void showMessage(String msg, String subMsg);
const unsigned char* getIcon(int code);

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n--- BOOTING (Forecast Ver) ---");
    
    pinMode(BTN_PIN, INPUT_PULLUP);
    pinMode(16, OUTPUT); digitalWrite(16, HIGH);
    pinMode(2, OUTPUT);  digitalWrite(2, HIGH); 
    pinMode(BTN_PIN, INPUT_PULLUP);

    display.init(115200, true, 2, true);
    display.setRotation(0);
    display.setTextColor(GxEPD_BLACK);
    
    WiFiManager wm;
    wm.setConnectTimeout(10);
    if (!wm.autoConnect("ESP-EPAPER-SETUP")) {
        Serial.println("Offline Mode");
    }
    
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
    if (WiFi.status() == WL_CONNECTED) {
        fetchWeather();
    }
    
    drawScreen();
}

void loop() {
    // --- BUTTON ---
    int reading = digitalRead(BTN_PIN);
    if (reading == LOW && lastBtnState == HIGH) {
        pressStartTime = millis();
        isPressing = true;
    }
    if (reading == HIGH && lastBtnState == LOW) {
        unsigned long duration = millis() - pressStartTime;
        isPressing = false;
        
        if (duration < 3000) {
            if (WiFi.status() == WL_CONNECTED) fetchWeather();
            drawScreen();
            lastWeatherUpdate = millis();
        } else if (duration < 10000) {
            performOTA();
        } else {
            enterConfigMode();
        }
    }
    lastBtnState = reading;

    // --- AUTO ---
    unsigned long now = millis();
    if (!isPressing) {
        if (now - lastTimeUpdate > TIME_UPDATE_INTERVAL) {
            lastTimeUpdate = now;
            drawScreen();
        }
        
        if (now - lastWeatherUpdate > WEATHER_UPDATE_INTERVAL) {
            lastWeatherUpdate = now;
            if (WiFi.status() == WL_CONNECTED) fetchWeather();
        }
    }
    delay(50);
}

const unsigned char* getIcon(int code) {
    if (code <= 1) return icon_sun_64;
    if (code <= 48) return icon_cloud_64; 
    if (code >= 95) return icon_rain_64;  
    if (code >= 51) return icon_rain_64;  
    return icon_cloud_64;
}

void drawScreen() {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    char timeBuf[10];
    char dateBuf[30];
    
    if (timeinfo->tm_year < 100) {
        strcpy(timeBuf, "--:--");
        strcpy(dateBuf, "Syncing...");
    } else {
        strftime(timeBuf, 10, "%H:%M", timeinfo);
        strftime(dateBuf, 30, "%A, %d/%m", timeinfo);
    }

    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        
        // --- 1. CLOCK (HUGE & CENTERED) ---
        // Prioritize Clock as requested
        display.setFont(&FreeSansBold24pt7b);
        display.setTextSize(2); // Double size for visibility
        
        int16_t tbx, tby; uint16_t tbw, tbh;
        display.getTextBounds(timeBuf, 0, 0, &tbx, &tby, &tbw, &tbh);
        int xTime = (640 - tbw) / 2;
        int yTime = 130;  
        display.setCursor(xTime, yTime);
        display.print(timeBuf);
        display.setTextSize(1); // Reset size

        // --- 2. DATE (Below Clock) ---
        display.setFont(&FreeSans12pt7b);
        display.getTextBounds(dateBuf, 0, 0, &tbx, &tby, &tbw, &tbh);
        int xDate = (640 - tbw) / 2;
        int yDate = 170;
        display.setCursor(xDate, yDate);
        display.print(dateBuf);
        
        // --- 3. CURRENT WEATHER (Below Date) ---
        if (weather.valid) {
            String tempStr = String((int)weather.currentTemp) + " C";
            display.setFont(&FreeSansBold18pt7b);
            display.getTextBounds(tempStr.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
            
            int totalW = 64 + 10 + tbw; // Icon(64) + Gap(10) + Text
            int startX = (640 - totalW) / 2;
            
            // Icon
            display.drawBitmap(startX, 190, getIcon(weather.currentCode), 64, 64, GxEPD_BLACK);
            
            // Temp (Aligned w/ Icon)
            display.setCursor(startX + 74, 240); 
            display.print(tempStr);
        } else {
            display.setFont(&FreeSans9pt7b);
            display.setCursor(260, 220);
            display.print("No Weather Data");
        }

        // --- DIVIDER (Lowered for Priority) ---
        int divY = 265;
        display.drawLine(10, divY, 630, divY, GxEPD_BLACK);

        // --- 4. FORECAST (Compact Footer) ---
        // Shifted down to give more room to Clock
        int colWidth = 155;
        int footerY = divY + 5;
        
        if (weather.valid) {
            for(int i=0; i<4; i++) {
                int x = 10 + (i * colWidth);
                int center = x + (colWidth/2);
                
                // Day
                char dayName[10];
                struct tm* dtm = localtime(&weather.daily[i].date);
                strftime(dayName, 10, "%a", dtm); 
                
                display.setFont(&FreeSans9pt7b); // Smaller font
                display.getTextBounds(dayName, 0, 0, &tbx, &tby, &tbw, &tbh);
                display.setCursor(center - (tbw/2), footerY + 20);
                display.print(dayName);
                
                // Icon (64x64)
                display.drawBitmap(center - 32, footerY + 25, getIcon(weather.daily[i].code), 64, 64, GxEPD_BLACK);
                
                // Temp Range
                String t = String((int)weather.daily[i].maxTemp) + "/" + String((int)weather.daily[i].minTemp);
                display.getTextBounds(t.c_str(), 0, 0, &tbx, &tby, &tbw, &tbh);
                display.setCursor(center - (tbw/2), footerY + 105);
                display.print(t);
            }
        }
    } while (display.nextPage());
    display.powerOff();
}

void fetchWeather() {
    if (WiFi.status() != WL_CONNECTED) return;
    
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    
    String url = "https://api.open-meteo.com/v1/forecast?latitude=" + String(LAT) + "&longitude=" + String(LON) + 
                 "&daily=weathercode,temperature_2m_max,temperature_2m_min&current_weather=true&timezone=Asia%2FBangkok&forecast_days=4";
                 
    if (http.begin(client, url)) {
        if (http.GET() > 0) {
            JsonDocument doc; 
            DeserializationError error = deserializeJson(doc, http.getString());
            
            if (!error) {
                weather.currentTemp = doc["current_weather"]["temperature"];
                weather.currentCode = doc["current_weather"]["weathercode"];
                
                JsonArray codeArr = doc["daily"]["weathercode"];
                JsonArray maxArr = doc["daily"]["temperature_2m_max"];
                JsonArray minArr = doc["daily"]["temperature_2m_min"];
                
                time_t now = time(nullptr); 
                // Adjust if time not synced well, but assuming sync:
                
                for(int i=0; i<4; i++) {
                    weather.daily[i].date = now + (i * 86400); 
                    weather.daily[i].code = codeArr[i];
                    weather.daily[i].maxTemp = maxArr[i];
                    weather.daily[i].minTemp = minArr[i];
                }
                weather.valid = true;
            } else {
                Serial.print("Json Error: "); Serial.println(error.c_str());
            }
        }
        http.end();
    }
}

void performOTA() {
    showMessage("OTA UPDATE", "Keep Power On!");
    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(10000);
    ESPhttpUpdate.setLedPin(2, HIGH);
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, FIRMWARE_URL);
    if (ret == HTTP_UPDATE_FAILED) {
        showMessage("OTA FAIL", ESPhttpUpdate.getLastErrorString());
        delay(3000);
    }
    ESP.restart();
}

void enterConfigMode() {
    showMessage("WIFI SETUP", "AP: ESP-EPAPER-SETUP");
    WiFiManager wm;
    if (!wm.startConfigPortal("ESP-EPAPER-SETUP")) {
        ESP.restart();
    }
    ESP.restart();
}

void showMessage(String msg, String subMsg) {
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setFont(&FreeSansBold24pt7b);
        int16_t bx, by; uint16_t bw, bh;
        display.getTextBounds(msg.c_str(), 0, 0, &bx, &by, &bw, &bh);
        display.setCursor((640-bw)/2, 150);
        display.print(msg);
        
        display.setFont(&FreeSans12pt7b);
        display.getTextBounds(subMsg.c_str(), 0, 0, &bx, &by, &bw, &bh);
        display.setCursor((640-bw)/2, 250);
        display.print(subMsg);
    } while (display.nextPage());
}
