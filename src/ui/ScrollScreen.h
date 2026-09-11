// ============================================================================
//  ScrollScreen.h  -  Reusable read-only, scrollable line output.
//
//  Tools append coloured lines and (optionally) show a "busy" banner while
//  working. One-button use: Next pages down (wraps at the end), Back exits.
//  Subclasses do work in update() and call addLine(); the base handles all
//  layout, scrolling and drawing.
// ============================================================================
#pragma once

#include <vector>
#include <Arduino.h>
#include "ui/Screen.h"
#include "ui/Theme.h"

class ScrollScreen : public Screen {
public:
    explicit ScrollScreen(const char* title) : _title(title) {}

    const char* title() const override { return _title; }
    void onEnter() override { markDirty(); }

    void setTitle(const char* t) { _title = t; markDirty(); }
    void resetLines();
    void addLine(const String& s, uint16_t color = theme::TEXT);
    void setBusy(bool busy, const char* banner = nullptr);
    bool busy() const { return _busy; }

    bool handle(InputEvent e) override;
    void draw(Display& d) override;
    bool animated() const override { return _busy; }

protected:
    const char* footerHint() const;

private:
    struct Line { String text; uint16_t color; };
    static constexpr size_t MAX_LINES = 200;

    const char* _title;
    std::vector<Line> _lines;
    int  _top = 0;
    bool _follow = true;          // auto-scroll to newest while at the bottom
    bool _busy = false;
    const char* _banner = nullptr;

    int visibleRows() const;
    int maxTop() const;
};
