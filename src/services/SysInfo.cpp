#include "services/SysInfo.h"
#include "services/Sd.h"
#include "Config.h"
#include <esp_mac.h>

namespace sysinfo {

static String macStr(esp_mac_type_t type)
{
    uint8_t m[6] = {0};
    if (esp_read_mac(m, type) != ESP_OK) return String("--");
    char b[18];
    snprintf(b, sizeof(b), "%02X:%02X:%02X:%02X:%02X:%02X",
             m[0], m[1], m[2], m[3], m[4], m[5]);
    return String(b);
}

String uptimeStr()
{
    uint32_t s = millis() / 1000;
    uint32_t h = s / 3600; s %= 3600;
    uint32_t m = s / 60;   s %= 60;
    char b[16];
    snprintf(b, sizeof(b), "%luh%02lum%02lus", (unsigned long)h, (unsigned long)m, (unsigned long)s);
    return String(b);
}

float temperatureC()
{
    return temperatureRead();     // on-die sensor, degrees C
}

std::vector<KV> snapshot()
{
    std::vector<KV> v;
    char buf[40];

    snprintf(buf, sizeof(buf), "%s r%d", ESP.getChipModel(), ESP.getChipRevision());
    v.push_back({"Chip", buf});

    snprintf(buf, sizeof(buf), "%d core @ %luMHz", ESP.getChipCores(),
             (unsigned long)ESP.getCpuFreqMHz());
    v.push_back({"CPU", buf});

    v.push_back({"SDK", ESP.getSdkVersion()});

    snprintf(buf, sizeof(buf), "%.1f C", temperatureC());
    v.push_back({"Temp", buf});

    v.push_back({"Uptime", uptimeStr()});

    snprintf(buf, sizeof(buf), "%u/%u KB free",
             (unsigned)(ESP.getFreeHeap() / 1024), (unsigned)(ESP.getHeapSize() / 1024));
    v.push_back({"Heap", buf});

    snprintf(buf, sizeof(buf), "%u KB min", (unsigned)(ESP.getMinFreeHeap() / 1024));
    v.push_back({"Heap low", buf});

    size_t psram = ESP.getPsramSize();
    v.push_back({"PSRAM", psram ? sd::humanSize(psram) : String("none")});

    snprintf(buf, sizeof(buf), "%u MB", (unsigned)(ESP.getFlashChipSize() / (1024 * 1024)));
    v.push_back({"Flash", buf});

    snprintf(buf, sizeof(buf), "%u/%u KB",
             (unsigned)(ESP.getSketchSize() / 1024),
             (unsigned)((ESP.getSketchSize() + ESP.getFreeSketchSpace()) / 1024));
    v.push_back({"Sketch", buf});

    v.push_back({"WiFi MAC", macStr(ESP_MAC_WIFI_STA)});
    v.push_back({"BT MAC",   macStr(ESP_MAC_BT)});

    if (sd::mounted()) {
        String s = String(sd::typeStr()) + " " + sd::humanSize(sd::sizeBytes());
        v.push_back({"SD card", s});
    } else {
        v.push_back({"SD card", "not present"});
    }

    return v;
}

} // namespace sysinfo
