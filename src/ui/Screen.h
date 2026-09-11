// ============================================================================
//  Screen.h  -  Base class for every UI screen.
//
//  Lifecycle (driven by UiManager):
//    onEnter()  when the screen becomes active (pushed, or revealed by a pop)
//    onExit()   when it is left
//    update()   every frame, for non-drawing background work (polling a scan)
//    handle()   on a button gesture; return true if consumed. If BACK is not
//               consumed, UiManager pops this screen.
//    draw()     render into the framebuffer when dirty()
//
//  Screens are long-lived singletons owned by the app, so no heap churn.
// ============================================================================
#pragma once

#include "hal/Display.h"
#include "hal/Button.h"

class UiManager;

class Screen {
public:
    virtual ~Screen() {}

    virtual const char* title() const { return ""; }

    virtual void onEnter() { markDirty(); }
    virtual void onExit()  {}
    virtual void update()  {}
    virtual bool handle(InputEvent e) { (void)e; return false; }
    virtual void draw(Display& d) = 0;

    bool dirty() const   { return _dirty; }
    void markDirty()     { _dirty = true; }
    void clearDirty()    { _dirty = false; }

    // Redraw every frame regardless of dirty flag (for live/animated screens).
    virtual bool animated() const { return false; }

    void attach(UiManager* m) { _ui = m; }

protected:
    UiManager* _ui = nullptr;
    bool _dirty = true;
};
