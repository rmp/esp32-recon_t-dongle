#include "hal/StatusLed.h"
#include "Config.h"

#define FASTLED_INTERNAL          // silence the pragma version banner
#include <FastLED.h>

static CRGB s_led[1];

void StatusLed::begin()
{
    FastLED.addLeds<APA102, PIN_LED_DI, PIN_LED_CI, BGR>(s_led, 1);
    FastLED.setBrightness(40);    // keep it modest; this LED is bright
    set(LedState::Idle);
}

void StatusLed::render(uint8_t r, uint8_t g, uint8_t b)
{
    s_led[0] = CRGB(r, g, b);
    FastLED.show();
}

void StatusLed::set(LedState s)
{
    _state = s;
    _phase = 0;
    _nextMs = 0;                  // force immediate refresh on next tick
    tick();
}

void StatusLed::tick()
{
    const uint32_t now = millis();
    if (now < _nextMs) return;

    switch (_state) {
        case LedState::Idle:
            render(0, 24, 32);            // dim cyan
            _nextMs = now + 500;
            break;
        case LedState::Ok:
            render(0, 180, 0);            // green
            _nextMs = now + 500;
            break;
        case LedState::Warn:
            render(180, 140, 0);          // yellow
            _nextMs = now + 500;
            break;
        case LedState::Error:
            render(200, 0, 0);            // red
            _nextMs = now + 500;
            break;
        case LedState::Busy: {
            uint8_t v = (_phase < 16) ? _phase * 12 : (32 - _phase) * 12;
            render(v, v / 2, 0);          // amber breathe
            _phase = (_phase + 1) & 31;
            _nextMs = now + 40;
            break;
        }
        case LedState::Active: {
            uint8_t v = (_phase < 16) ? _phase * 14 : (32 - _phase) * 14;
            render(v, 0, v);              // magenta breathe (intrusive op live)
            _phase = (_phase + 1) & 31;
            _nextMs = now + 30;
            break;
        }
        case LedState::Off:
        default:
            render(0, 0, 0);
            _nextMs = now + 1000;
            break;
    }
}
