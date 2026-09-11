// ============================================================================
//  MenuScreen.h  -  Generic single-column scrollable menu.
//
//  Build a menu by adding items, each with a callback. Navigation with the one
//  button: Next scrolls the highlight, Select fires the item's callback, Back
//  is left unconsumed so the UiManager pops to the parent.
// ============================================================================
#pragma once

#include <functional>
#include <vector>
#include "ui/Screen.h"

class MenuScreen : public Screen {
public:
    using Action = std::function<void()>;

    explicit MenuScreen(const char* title, const char* footer = "click:next  hold:select")
        : _title(title), _footer(footer) {}

    void addItem(const char* label, Action cb) { _items.push_back({label, std::move(cb)}); }
    void clear() { _items.clear(); _sel = 0; _top = 0; markDirty(); }

    const char* title() const override { return _title; }
    void onEnter() override { markDirty(); }
    bool handle(InputEvent e) override;
    void draw(Display& d) override;

private:
    struct Item { const char* label; Action cb; };
    const char* _title;
    const char* _footer;
    std::vector<Item> _items;
    int _sel = 0;
    int _top = 0;

    int visibleRows() const;
    void ensureVisible();
};
