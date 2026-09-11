// ============================================================================
//  SystemInfoScreen.h  -  Live device facts (chip, memory, flash, MACs, temp,
//  uptime, SD). Refreshes once a second without disturbing the scroll position.
// ============================================================================
#pragma once

#include <vector>
#include "ui/Screen.h"
#include "services/SysInfo.h"

class SystemInfoScreen : public Screen {
public:
    const char* title() const override { return "System Info"; }
    void onEnter() override;
    void update()  override;
    bool handle(InputEvent e) override;
    void draw(Display& d) override;
    bool animated() const override { return true; }

private:
    std::vector<sysinfo::KV> _rows;
    int _top = 0;
    uint32_t _lastRefresh = 0;
    void refresh();
    int visibleRows() const;
    int maxTop() const;
};
