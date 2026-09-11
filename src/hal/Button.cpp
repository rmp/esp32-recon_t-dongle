#include "hal/Button.h"
#include "Config.h"

void Button::begin(uint8_t pin)
{
    _pin = pin;
    pinMode(_pin, INPUT_PULLUP);
    _raw = _down = false;
    _rawSince = millis();
}

uint16_t Button::holdMs() const
{
    if (!_down) return 0;
    uint32_t d = millis() - _downAt;
    return (d > 0xFFFF) ? 0xFFFF : (uint16_t)d;
}

Arm Button::arming() const
{
    uint16_t h = holdMs();
    if (h >= GESTURE_BACK_MS)   return Arm::Back;
    if (h >= GESTURE_SELECT_MS) return Arm::Select;
    return Arm::None;
}

InputEvent Button::poll()
{
    const uint32_t now = millis();
    const bool pressed = (digitalRead(_pin) == LOW);   // active LOW

    // Track raw transitions for debouncing.
    if (pressed != _raw) {
        _raw = pressed;
        _rawSince = now;
    }

    // Only accept a stable level after the debounce window.
    if ((now - _rawSince) < GESTURE_DEBOUNCE_MS) {
        return InputEvent::None;
    }

    if (_raw && !_down) {
        // Debounced press begins.
        _down   = true;
        _downAt = _rawSince;
        return InputEvent::None;
    }

    if (!_raw && _down) {
        // Debounced release: classify by how long it was held.
        _down = false;
        const uint32_t held = now - _downAt;
        if (held >= GESTURE_BACK_MS)        return InputEvent::Back;
        if (held >= GESTURE_SELECT_MS)      return InputEvent::Select;
        return InputEvent::Next;            // short click
    }

    return InputEvent::None;
}
