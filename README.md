# ESP8266 7.5" E-Paper Information Display

A smart information display firmware for ESP8266 interacting with a 7.5-inch E-Ink screen. It features a large digital clock, real-time weather updates, multi-day forecasts, and Over-The-Air (OTA) firmware updates.

## 🌟 Features
- **Large Digital Clock**: Huge, legible time display with date (Day, DD/MM).
- **Weather Station**:
    - Current Temperature & Weather Condition (Large Icon).
    - 4-Day Forecast (Day Name, Icon, Temp Range).
    - Data Source: Open-Meteo API (No API Key required).
- **Smart Button Control** (Single Button on GPIO 2):
    - **Short Press (< 3s)**: Refresh Screen immediately.
    - **Medium Press (3s - 10s)**: Trigger OTA Firmware Update.
    - **Long Press (> 10s)**: Enter WiFi Configuration Mode (AP Mode).
- **Auto-Updates**:
    - Clock: Every 1 minute.
    - Weather: Every 30 minutes.
- **WiFi Manager**: Easy configuration portal for connecting to WiFi without hardcoding credentials.
- **OTA Updates**: Check and update firmware from a remote GitHub URL.

## 🛠 Hardware Wiring

| ESP8266 Pin | E-Paper Pin | Description      |
| :---        | :---        | :---             |
| GPIO 13     | DIN         | SPI MOSI         |
| GPIO 14     | CLK         | SPI SCK          |
| GPIO 15     | CS          | Chip Select      |
| GPIO 4      | DC          | Data/Command     |
| GPIO 5      | RST         | Reset            |
| GPIO 12     | BUSY        | Busy Signal      |
| GPIO 2      | BUTTON      | Smart Switch     |
| 3.3V        | VCC         | Power (3.3V)     |
| GND         | GND         | Ground           |

**Note**: GPIO 2 (Switch) connects to GND when pressed.

## ⚠️ Critical Flashing Instruction (Boot Conflict)
**GPIO 2 is a Bootstrap Pin.** It MUST be HIGH (floating or pulled up) during boot for the ESP8266 to start correctly.
If you have a button connected between GPIO 2 and GND, it might interfere with the flashing/upload process.

**To Upload Code:**
1. **Unplug/Disconnect** the wire connected to **GPIO 2**.
2. Connect ESP8266 to PC via USB.
3. Run **Upload** task in PlatformIO.
4. Wait for `[SUCCESS]`.
5. **Reconnect** the wire to GPIO 2.

## 🚀 Usage Guide

### First Time Setup
1. Power on the device.
2. If no WiFi is configured, the device will create a hotspot named **`ESP-EPAPER-SETUP`**.
3. Connect to this hotspot with your phone/laptop.
4. A portal should open (or go to `192.168.4.1`).
5. Scan and Select your WiFi, enter password, and Save.
6. Device will restart and connect.

### Operation
- **Standard**: Displays Time & Weather. Updates automatically.
- **Refresh**: Click button once to force a screen refresh.
- **Update Firmware**: Hold button for 5 seconds (until you count to 5) then release. Screen will show "OTA UPDATE".
- **Reset WiFi**: Hold button for 10+ seconds. Screen will show "WIFI SETUP".

## 📦 Dependencies
- [GxEPD2](https://github.com/ZinggJM/GxEPD2) - E-Paper Driver
- [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library) - Graphics
- [ArduinoJson](https://arduinojson.org/) - JSON Parsing
- [WiFiManager](https://github.com/tzapu/WiFiManager) - WiFi Config
- [ESP8266HTTPClient](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266HTTPClient)
- [ESP8266httpUpdate](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266httpUpdate)

## 🔧 Configuration
Settings are located in `src/main.cpp`:
```cpp
const float LAT = 20.97;  // Your Latitude
const float LON = 105.85; // Your Longitude
const char* FIRMWARE_URL = "..."; // URL for OTA .bin file
```

## 📝 License
This project is open source.
