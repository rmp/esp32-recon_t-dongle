// ============================================================================
//  StatusLed.h  -  Single APA102 RGB LED used as a system status indicator.
//
//  Semantic states keep the meaning consistent across the whole app:
//    Idle     dim cyan     - at a menu, nothing running
//    Busy     amber pulse  - a scan / operation is in progress
//    Ok       green        - last operation succeeded
//    Warn     yellow       - attention / partial result
//    Error    red          - last operation failed
//    Active   magenta pulse- an intrusive action (HID injection, MSC) is live
// ============================================================================
#pragma once

#include <Arduino.h>

enum class LedState : uint8_t { Idle, Busy, Ok, Warn, Error, Active, Off };

class StatusLed {
public:
    void begin();
    void set(LedState s);
    void tick();                 // call regularly to animate pulses
    LedState state() const { return _state; }

private:
    LedState _state = LedState::Idle;
    uint32_t _nextMs = 0;
    uint8_t  _phase  = 0;
    void render(uint8_t r, uint8_t g, uint8_t b);
};
