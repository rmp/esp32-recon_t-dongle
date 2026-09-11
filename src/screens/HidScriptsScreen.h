// ============================================================================
//  HidScriptsScreen.h  -  Selectable list of DuckyScript payloads found on the
//  SD card (/payloads/*.txt|*.dd). Selecting one invokes onRun (wired by main
//  to execute it as a HID payload via the shared ToolScreen).
// ============================================================================
#pragma once

#include <vector>
#include <functional>
#include "ui/Screen.h"

class HidScriptsScreen : public Screen {
public:
    const char* title() const override { return "Payloads"; }

    void setDir(const char* dir) { _dir = dir; }
    void setOnRun(std::function<void(const String&)> cb) { _onRun = std::move(cb); }

    void onEnter() override;
    bool handle(InputEvent e) override;
    void draw(Display& d) override;

private:
    const char* _dir = "/payloads";
    std::vector<String> _files;
    int _sel = 0;
    int _top = 0;
    std::function<void(const String&)> _onRun;

    int visibleRows() const;
    void ensureVisible();
};
