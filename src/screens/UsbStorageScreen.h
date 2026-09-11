// ============================================================================
//  UsbStorageScreen.h  -  Expose / hide the microSD card as a USB drive.
//
//  Select toggles exposure. For safety the card is automatically hidden again
//  when leaving this screen, so on-device SD tools never race the host.
// ============================================================================
#pragma once

#include "ui/Screen.h"

class UsbStorageScreen : public Screen {
public:
    const char* title() const override { return "USB Storage"; }
    void onEnter() override;
    void onExit()  override;
    bool handle(InputEvent e) override;
    void draw(Display& d) override;
    bool animated() const override { return true; }
};
