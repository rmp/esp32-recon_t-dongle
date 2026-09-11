#include "services/BleService.h"
#include "ui/Theme.h"

#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

namespace ble {

static bool s_inited = false;

void scan(Worker& w, uint32_t seconds)
{
    if (!s_inited) {
        BLEDevice::init("");
        s_inited = true;
    }

    BLEScan* scanner = BLEDevice::getScan();
    scanner->setActiveScan(true);
    scanner->setInterval(100);
    scanner->setWindow(99);

    w.emit("BLE scanning " + String(seconds) + "s...", theme::ACCENT);

    BLEScanResults results = scanner->start(seconds, false);
    int n = results.getCount();

    for (int i = 0; i < n && !w.stopRequested(); ++i) {
        BLEAdvertisedDevice d = results.getDevice(i);
        String addr = d.getAddress().toString().c_str();
        int    rssi = d.getRSSI();
        int    bars = 0;
        if (rssi >= -60) bars = 4; else if (rssi >= -72) bars = 3;
        else if (rssi >= -84) bars = 2; else if (rssi >= -95) bars = 1;

        uint16_t col = bars >= 3 ? theme::OK : (bars >= 2 ? theme::TEXT : theme::TEXT_DIM);
        w.emit(addr + "  " + String(rssi) + "dBm", col);

        String detail;
        if (d.haveName())        detail += String(d.getName().c_str());
        if (d.haveServiceUUID()) {
            if (detail.length()) detail += " ";
            detail += "[" + String(d.getServiceUUID().toString().c_str()) + "]";
        }
        if (detail.length()) w.emit("  " + detail, theme::TEXT_DIM);
    }

    scanner->clearResults();
    w.emit("Found " + String(n) + " device(s)", theme::ACCENT);
}

} // namespace ble
