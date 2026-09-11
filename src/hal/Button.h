// ============================================================================
//  Button.h  -  One-button gesture detector for the BOOT button (GPIO0).
//
//  The T-Dongle-S3 has only this single user input, so the whole UI is driven
//  by three gestures decided at release time:
//     short click        -> Next
//     long press ~0.6s    -> Select
//     very-long ~2s       -> Back
//
//  While the button is held, arming() / holdMs() let the UI render a progress
//  bar showing which action will fire on release.
// ============================================================================
#pragma once

#include <Arduino.h>

enum class InputEvent : uint8_t { None, Next, Select, Back };
enum class Arm        : uint8_t { None, Select, Back };

class Button {
public:
    void begin(uint8_t pin);

    // Call every loop. Returns an event once, on release.
    InputEvent poll();

    bool     isHeld()  const { return _down; }
    uint16_t holdMs()  const;          // 0 when not held
    Arm      arming()  const;          // action currently armed while holding

private:
    uint8_t  _pin      = 0;
    bool     _down     = false;        // debounced logical state (pressed)
    bool     _raw      = false;        // last raw sample
    uint32_t _rawSince = 0;            // when raw last changed
    uint32_t _downAt   = 0;            // debounced press start
};
