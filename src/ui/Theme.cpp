#include "ui/Theme.h"
#include "Config.h"

namespace theme {

void drawHeader(TFT_eSprite& g, const char* title, int pos, int count)
{
    g.fillRect(0, 0, SCREEN_W, HEADER_H, HEADER_BG);
    g.setTextDatum(TL_DATUM);
    g.setTextColor(HEADER_FG, HEADER_BG);
    g.setTextFont(1);
    g.drawString(title, PAD, 4);

    if (pos >= 0 && count > 0) {
        char buf[12];
        snprintf(buf, sizeof(buf), "%d/%d", pos + 1, count);
        g.setTextDatum(TR_DATUM);
        g.setTextColor(ACCENT, HEADER_BG);
        g.drawString(buf, SCREEN_W - PAD, 4);
        g.setTextDatum(TL_DATUM);
    }
}

void drawFooter(TFT_eSprite& g, const char* hint)
{
    const int16_t y = SCREEN_H - FOOTER_H;
    g.fillRect(0, y, SCREEN_W, FOOTER_H, HEADER_BG);
    g.setTextDatum(TL_DATUM);
    g.setTextColor(TEXT_DIM, HEADER_BG);
    g.setTextFont(1);
    g.drawString(hint, PAD, y + 2);
}

bool drawHoldOverlay(TFT_eSprite& g, Arm arm, uint16_t holdMs)
{
    if (arm == Arm::None) return false;

    const int16_t y = SCREEN_H - FOOTER_H;
    uint16_t col   = (arm == Arm::Back) ? ERR : OK;
    const char* lbl = (arm == Arm::Back) ? "release: BACK" : "release: SELECT";

    // Progress fraction across the whole gesture range.
    uint16_t span = GESTURE_MAX_MS;
    uint16_t clamped = holdMs > span ? span : holdMs;
    int16_t w = (int32_t)clamped * SCREEN_W / span;

    g.fillRect(0, y, SCREEN_W, FOOTER_H, theme::BG);
    g.fillRect(0, y, w, FOOTER_H, col);
    g.setTextDatum(TL_DATUM);
    g.setTextColor(HEADER_FG);
    g.setTextFont(1);
    g.drawString(lbl, PAD, y + 2);
    return true;
}

void drawListRow(TFT_eSprite& g, int16_t y, const char* text,
                 bool selected, const char* right, uint16_t fg)
{
    if (selected) {
        g.fillRect(0, y, SCREEN_W, ROW_H, SEL_BG);
        g.setTextColor(SEL_FG, SEL_BG);
    } else {
        g.setTextColor(fg, BG);
    }
    g.setTextFont(1);
    g.setTextDatum(TL_DATUM);
    g.drawString(text, PAD + (selected ? 2 : 0), y + 2);

    if (right && right[0]) {
        g.setTextDatum(TR_DATUM);
        if (!selected) g.setTextColor(TEXT_DIM, BG);
        g.drawString(right, SCREEN_W - PAD, y + 2);
        g.setTextDatum(TL_DATUM);
    }
}

} // namespace theme
