// ============================================================================
//  WifiScanScreen.h  -  Asynchronous Wi-Fi AP scan with a selectable result
//  list. Row 0 is "Rescan"; selecting an AP invokes onSelect (wired by main to
//  show details). Non-blocking: the scan runs in the WiFi driver, polled here.
// ============================================================================
#pragma once

#include <vector>
#include <functional>
#include "ui/Screen.h"
#include "services/WifiService.h"

class WifiScanScreen : public Screen {
public:
    const char* title() const override { return "WiFi APs"; }

    void setOnSelect(std::function<void(const wifi::Ap&)> cb) { _onSelect = std::move(cb); }

    void onEnter() override;
    void update()  override;
    bool handle(InputEvent e) override;
    void draw(Display& d) override;
    bool animated() const override { return _scanning; }

private:
    std::vector<wifi::Ap> _aps;
    bool _scanning = false;
    int  _sel = 0;                 // 0 = Rescan, 1.. = AP index+1
    int  _top = 0;
    std::function<void(const wifi::Ap&)> _onSelect;

    int itemCount() const { return 1 + (int)_aps.size(); }
    int visibleRows() const;
    void ensureVisible();
    void startScan();
};
