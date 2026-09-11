#include "hal/Display.h"
#include "esp_idf_version.h"
#include "driver/spi_master.h"

static constexpr uint8_t BL_LEDC_CH = 7;

static constexpr uint8_t LCD_MOSI = 3;
static constexpr uint8_t LCD_CLK  = 5;
static constexpr uint8_t LCD_CS   = 4;
static constexpr uint8_t LCD_DC   = 2;
static constexpr uint8_t LCD_RST  = 1;

bool Display::begin()
{
    // --- Hardware panel via esp_lcd API (proven working) ---
    spi_bus_config_t bus_cfg = ST7735_PANEL_BUS_SPI_CONFIG(
        LCD_CLK, LCD_MOSI, SCREEN_W * SCREEN_H * sizeof(uint16_t));
    if (spi_bus_initialize((spi_host_device_t)SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO) != ESP_OK) {
        return false;
    }

    esp_lcd_panel_io_spi_config_t io_cfg = ST7735_PANEL_IO_SPI_CONFIG(LCD_CS, LCD_DC, NULL, NULL);
    if (esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_cfg, &_io) != ESP_OK) {
        return false;
    }

    esp_lcd_panel_dev_config_t pcfg = {};
    pcfg.reset_gpio_num = LCD_RST;
    pcfg.color_space    = ESP_LCD_COLOR_SPACE_BGR;
    pcfg.bits_per_pixel = 16;
    if (esp_lcd_new_panel_st7735(_io, &pcfg, &_panel) != ESP_OK) {
        return false;
    }

    esp_lcd_panel_reset(_panel);
    esp_lcd_panel_init(_panel);
    esp_lcd_panel_invert_color(_panel, true);
    esp_lcd_panel_set_gap(_panel, 1, 26);
    esp_lcd_panel_swap_xy(_panel, true);
    esp_lcd_panel_mirror(_panel, false, true);
    esp_lcd_panel_disp_on_off(_panel, true);

    // --- In-memory framebuffer (TFT_eSprite as pure RAM canvas) ---
    _fb.setColorDepth(16);
    if (!_fb.createSprite(SCREEN_W, SCREEN_H)) {
        return false;
    }
    _fb.fillSprite(TFT_BLACK);
    _fb.setTextDatum(TL_DATUM);

    // --- Backlight via LEDC PWM (active LOW) ---
    pinMode(PIN_TFT_BL, OUTPUT);
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    ledcAttach(PIN_TFT_BL, 2000, 8);
#else
    ledcSetup(BL_LEDC_CH, 2000, 8);
    ledcAttachPin(PIN_TFT_BL, BL_LEDC_CH);
#endif
    setBacklight(255);

    _ready = true;
    return true;
}

void Display::flush()
{
    if (!_ready || !_panel) return;
    uint16_t* buf = (uint16_t*)_fb.getPointer();
    if (!buf) return;
    esp_lcd_panel_draw_bitmap(_panel, 0, 0, SCREEN_W, SCREEN_H, buf);
}

void Display::setBacklight(uint8_t level)
{
    const uint8_t duty = 255 - level;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    ledcWrite(PIN_TFT_BL, duty);
#else
    ledcWrite(BL_LEDC_CH, duty);
#endif
}
