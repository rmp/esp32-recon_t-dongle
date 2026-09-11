// ============================================================================
//  UiManager.h  -  Screen stack + input routing + frame pump.
//
//  Holds a stack of Screen*. The top screen is active. Gestures route to it;
//  an unconsumed BACK pops the stack (returns to the previous screen). Drawing
//  is done into the shared framebuffer and flushed at most every UI_FRAME_MS.
//  The one-button hold overlay is drawn on top of whatever the screen rendered.
// ============================================================================
#pragma once

#include "ui/Screen.h"
#include "hal/Display.h"
#include "hal/Button.h"
#include "hal/StatusLed.h"

class UiManager {
public:
    void begin(Display* d, Button* b, StatusLed* led);

    void push(Screen* s);
    void pop();
    Screen* top() const { return _n ? _stack[_n - 1] : nullptr; }
    int depth() const { return _n; }

    // Feed one gesture (or None) and render a frame.
    void loop();

    StatusLed* led() { return _led; }

private:
    static constexpr int MAX_DEPTH = 8;
    Screen*    _stack[MAX_DEPTH] = {nullptr};
    int        _n = 0;

    Display*   _d   = nullptr;
    Button*    _btn = nullptr;
    StatusLed* _led = nullptr;

    uint32_t _lastFrame = 0;
    Arm      _lastArm   = Arm::None;

    void render();
};
