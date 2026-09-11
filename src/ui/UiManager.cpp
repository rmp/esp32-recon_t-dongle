#include "ui/UiManager.h"
#include "ui/Theme.h"
#include "Config.h"

void UiManager::begin(Display* d, Button* b, StatusLed* led)
{
    _d = d; _btn = b; _led = led;
}

void UiManager::push(Screen* s)
{
    if (!s || _n >= MAX_DEPTH) return;
    if (_n) _stack[_n - 1]->onExit();
    s->attach(this);
    _stack[_n++] = s;
    s->onEnter();
}

void UiManager::pop()
{
    if (_n <= 1) return;                 // never pop the root screen
    _stack[--_n]->onExit();
    _stack[_n] = nullptr;
    _stack[_n - 1]->onEnter();           // revealed screen refreshes
}

void UiManager::loop()
{
    Screen* s = top();
    if (!s) return;

    // 1) Input.
    InputEvent e = _btn->poll();
    if (e != InputEvent::None) {
        bool consumed = s->handle(e);
        if (!consumed && e == InputEvent::Back) {
            pop();
        }
        s = top();
    }

    // 2) Background work.
    s->update();

    // 3) Frame pacing. Redraw when the screen is dirty, animated, or while the
    //    button is being held (so the hold overlay animates).
    Arm arm = _btn->arming();
    bool holdingChanged = (arm != _lastArm);
    _lastArm = arm;

    const uint32_t now = millis();
    bool due = (now - _lastFrame) >= UI_FRAME_MS;
    bool wantRedraw = s->dirty() || s->animated() || (arm != Arm::None) || holdingChanged;

    if (wantRedraw && due) {
        _lastFrame = now;
        render();
        s->clearDirty();
    }

    // 4) LED animation.
    _led->tick();
}

void UiManager::render()
{
    Screen* s = top();
    if (!s || !_d) return;

    TFT_eSprite& g = _d->canvas();
    g.fillSprite(theme::BG);
    s->draw(*_d);

    // One-button hold feedback overlays the footer area.
    theme::drawHoldOverlay(g, _btn->arming(), _btn->holdMs());

    _d->flush();
}
