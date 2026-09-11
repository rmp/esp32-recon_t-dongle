#include "services/WifiService.h"
#include "services/Sd.h"
#include <SD_MMC.h>

namespace wifi {

void startScan(bool showHidden)
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(false, false);
    WiFi.scanDelete();
    // async = true, show_hidden = showHidden
    WiFi.scanNetworks(true, showHidden);
}

int  scanState()  { return WiFi.scanComplete(); }
int  scanCount()  { int n = WiFi.scanComplete(); return n < 0 ? 0 : n; }
void clearScan()  { WiFi.scanDelete(); }

Ap ap(int i)
{
    Ap a;
    a.ssid    = WiFi.SSID(i);
    a.bssid   = WiFi.BSSIDstr(i);
    a.rssi    = WiFi.RSSI(i);
    a.channel = WiFi.channel(i);
    a.enc     = (uint8_t)WiFi.encryptionType(i);
    a.hidden  = (a.ssid.length() == 0);
    if (a.hidden) a.ssid = "<hidden>";
    return a;
}

const char* encStr(uint8_t enc)
{
    switch (enc) {
        case WIFI_AUTH_OPEN:            return "OPEN";
        case WIFI_AUTH_WEP:             return "WEP";
        case WIFI_AUTH_WPA_PSK:         return "WPA";
        case WIFI_AUTH_WPA2_PSK:        return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK:    return "WPA/2";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-E";
        case WIFI_AUTH_WPA3_PSK:        return "WPA3";
        case WIFI_AUTH_WPA2_WPA3_PSK:   return "WPA2/3";
        default:                        return "?";
    }
}

uint16_t rssiBars(int32_t rssi)
{
    if (rssi >= -55) return 4;
    if (rssi >= -67) return 3;
    if (rssi >= -78) return 2;
    if (rssi >= -90) return 1;
    return 0;
}

void connect(const char* ssid, const char* pass)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
}

void disconnect()  { WiFi.disconnect(true, false); }
bool isConnected() { return WiFi.status() == WL_CONNECTED; }

IfInfo ifInfo()
{
    IfInfo n;
    n.ip      = WiFi.localIP().toString();
    n.gw      = WiFi.gatewayIP().toString();
    n.mask    = WiFi.subnetMask().toString();
    n.dns     = WiFi.dnsIP().toString();
    n.mac     = WiFi.macAddress();
    n.ssid    = WiFi.SSID();
    n.rssi    = WiFi.RSSI();
    n.channel = WiFi.channel();
    return n;
}

int loadCreds(Cred* out, int maxN)
{
    if (!sd::mounted()) return 0;
    File f = SD_MMC.open("/wifi.txt", "r");
    if (!f) return 0;

    int n = 0;
    while (f.available() && n < maxN) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0 || line.startsWith("#")) continue;
        int sep = line.indexOf(',');
        if (sep <= 0) continue;
        out[n].ssid = line.substring(0, sep);
        out[n].pass = line.substring(sep + 1);
        out[n].ssid.trim();
        out[n].pass.trim();
        n++;
    }
    f.close();
    return n;
}

} // namespace wifi
