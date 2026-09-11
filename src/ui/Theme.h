// ============================================================================
//  Theme.h  -  Shared colours, layout metrics and common draw helpers so every
//  screen looks consistent and the look can be retuned in one place.
// ============================================================================
#pragma once

#include <TFT_eSPI.h>
#include "hal/Button.h"

namespace theme {

// ---- Palette (RGB565) ------------------------------------------------------
constexpr uint16_t BG        = 0x0000;   // black
constexpr uint16_t HEADER_BG = 0x018A;   // deep blue
constexpr uint16_t HEADER_FG = 0xFFFF;   // white
constexpr uint16_t TEXT      = 0xE71C;   // light grey
constexpr uint16_t TEXT_DIM  = 0x8410;   // grey
constexpr uint16_t SEL_BG    = 0x04BF;   // cyan-blue highlight
constexpr uint16_t SEL_FG    = 0xFFFF;   // white
constexpr uint16_t ACCENT    = 0x07FF;   // cyan
constexpr uint16_t OK        = 0x07E0;   // green
constexpr uint16_t WARN      = 0xFEE0;   // amber
constexpr uint16_t ERR       = 0xF800;   // red
constexpr uint16_t DANGER    = 0xF81F;   // magenta (intrusive actions)

// ---- Layout metrics --------------------------------------------------------
constexpr int16_t HEADER_H = 15;
constexpr int16_t FOOTER_H = 11;
constexpr int16_t ROW_H    = 13;         // list row height
constexpr int16_t PAD      = 3;

inline int16_t contentTop()    { return HEADER_H + 1; }
inline int16_t contentBottom(int16_t h) { return h - FOOTER_H - 1; }
inline int16_t contentHeight(int16_t h) { return contentBottom(h) - contentTop(); }

// ---- Common chrome ---------------------------------------------------------
// Title bar with optional "n/N" position indicator on the right.
void drawHeader(TFT_eSprite& g, const char* title, int pos = -1, int count = -1);

// Bottom hint strip (e.g. gesture legend).
void drawFooter(TFT_eSprite& g, const char* hint);

// While the button is held, overlay a progress bar showing the armed action.
// Returns true if it drew anything (i.e. the button is armed).
bool drawHoldOverlay(TFT_eSprite& g, Arm arm, uint16_t holdMs);

// One list row. `right` (optional) is right-aligned (e.g. RSSI / channel).
void drawListRow(TFT_eSprite& g, int16_t y, const char* text,
                 bool selected, const char* right = nullptr,
                 uint16_t fg = TEXT);

} // namespace theme
