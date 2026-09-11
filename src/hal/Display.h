#pragma once

#include <TFT_eSPI.h>
#include "Config.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "bsp_lcd/esp_lcd_st7735.h"

class Display {
public:
    bool begin();

    TFT_eSprite& canvas() { return _fb; }

    void flush();

    void setBacklight(uint8_t level);
    void backlightOn()  { setBacklight(255); }
    void backlightOff() { setBacklight(0); }

    int16_t width()  const { return SCREEN_W; }
    int16_t height() const { return SCREEN_H; }

private:
    TFT_eSPI    _tft = TFT_eSPI();
    TFT_eSprite _fb  = TFT_eSprite(&_tft);
    bool _ready = false;

    esp_lcd_panel_handle_t    _panel = NULL;
    esp_lcd_panel_io_handle_t _io    = NULL;
};
