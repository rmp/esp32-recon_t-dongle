#include "screens/AboutScreen.h"
#include "Config.h"

void AboutScreen::onEnter()
{
    resetLines();
    addLine(String(FW_NAME) + " v" + FW_VERSION, theme::ACCENT);
    addLine("LilyGo T-Dongle-S3", theme::TEXT);
    addLine("", theme::TEXT);
    addLine("One button:", theme::WARN);
    addLine(" click  = next", theme::TEXT);
    addLine(" hold   = select", theme::TEXT);
    addLine(" v.long = back", theme::TEXT);
    addLine("", theme::TEXT);
    addLine("Tools: WiFi/BLE recon,", theme::TEXT_DIM);
    addLine("net diag, USB HID,", theme::TEXT_DIM);
    addLine("USB storage.", theme::TEXT_DIM);
    addLine("", theme::TEXT);
    addLine("Authorized security", theme::DANGER);
    addLine("testing only.", theme::DANGER);
    markDirty();
}
