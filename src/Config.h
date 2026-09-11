// ============================================================================
//  Config.h  -  Central hardware map and tuning constants for the T-Dongle-S3.
//
//  Everything hardware- or UX-specific lives here so screens/services never
//  hard-code pins or magic numbers. See docs/en/t-dongle-s3 in the LilyGo repo
//  for the authoritative pin map.
// ============================================================================
#pragma once

#include <Arduino.h>

// ---- Firmware identity -----------------------------------------------------
#define FW_NAME     "T-Dongle Diag"
#define FW_VERSION  "0.1.0"

// ---- Display (ST7735 160x80, driven via esp_lcd_panel API) ----------------
static constexpr int16_t  SCREEN_W   = 160;   // landscape width  (rotation 1/3)
static constexpr int16_t  SCREEN_H   = 80;    // landscape height
static constexpr uint8_t  PIN_TFT_BL = 38;    // backlight, active LOW

// ---- RGB status LED (APA102 / SK9822, BGR order) ---------------------------
static constexpr uint8_t  PIN_LED_DI = 40;
static constexpr uint8_t  PIN_LED_CI = 39;

// ---- User button -----------------------------------------------------------
//  The T-Dongle-S3 has exactly ONE user button (BOOT on GPIO0). ALL navigation
//  must be possible with it. Active LOW with internal pull-up.
static constexpr uint8_t  PIN_BTN    = 0;

// ---- microSD (SD_MMC, 4-bit) -----------------------------------------------
static constexpr uint8_t  PIN_SD_CLK = 12;
static constexpr uint8_t  PIN_SD_CMD = 16;
static constexpr uint8_t  PIN_SD_D0  = 14;
static constexpr uint8_t  PIN_SD_D1  = 17;
static constexpr uint8_t  PIN_SD_D2  = 21;
static constexpr uint8_t  PIN_SD_D3  = 18;

// ---- QWIIC / Grove (UART by default) ---------------------------------------
static constexpr uint8_t  PIN_QWIIC_TX = 43;
static constexpr uint8_t  PIN_QWIIC_RX = 44;

// ============================================================================
//  One-button gesture timing (milliseconds).
//
//  Release-time decides the action:
//    press  <  GESTURE_SELECT_MS                 -> NEXT   (short click)
//    press  in [SELECT_MS, BACK_MS)              -> SELECT (long press ~0.6s)
//    press  >= GESTURE_BACK_MS                   -> BACK   (very-long ~2s)
//
//  While held, a progress bar shows the currently-armed action so the user
//  knows when to release for SELECT vs BACK.
// ============================================================================
static constexpr uint16_t GESTURE_DEBOUNCE_MS = 25;
static constexpr uint16_t GESTURE_SELECT_MS   = 600;
static constexpr uint16_t GESTURE_BACK_MS     = 2000;
static constexpr uint16_t GESTURE_MAX_MS      = 2600;  // cap for the progress bar

// ---- UI redraw cadence -----------------------------------------------------
static constexpr uint16_t UI_FRAME_MS = 33;   // ~30 fps ceiling for animations
