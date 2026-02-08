#ifndef DRIVER_CONFIG_H
#define DRIVER_CONFIG_H

// --- HARDWARE PINOUT ---
const int PIN_CS = 15;   // D8
const int PIN_DC = 4;    // D2
const int PIN_RST = 5;   // D1
const int PIN_BUSY = 12; // D6

const int PIN_SWITCH_1 = 2; // D4 - Toggle View

// --- DISPLAY DRIVER (640x384) ---
#define GxEPD2_DRIVER_CLASS GxEPD2_750
#define GxEPD2_BW_IS_Paged true

#endif
