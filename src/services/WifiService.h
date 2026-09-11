// ============================================================================
//  WifiService.h  -  Thin, non-blocking wrappers over the Arduino WiFi stack:
//  asynchronous AP scanning, station connect, and interface facts. Kept small
//  so screens stay declarative.
// ============================================================================
#pragma once

#include <Arduino.h>
#include <WiFi.h>

namespace wifi {

// ---- Scanning (asynchronous) ----------------------------------------------
void startScan(bool showHidden = true);
int  scanState();                 // WIFI_SCAN_RUNNING(-1), FAILED(-2), or count
int  scanCount();                 // valid once state >= 0
void clearScan();

struct Ap {
    String  ssid;
    String  bssid;
    int32_t rssi;
    int32_t channel;
    uint8_t enc;                  // wifi_auth_mode_t
    bool    hidden;
};
Ap  ap(int i);
const char* encStr(uint8_t enc);
uint16_t rssiBars(int32_t rssi);  // 0..4

// ---- Station connect -------------------------------------------------------
void connect(const char* ssid, const char* pass);
void disconnect();
bool isConnected();

struct IfInfo {
    String ip, gw, mask, dns, mac, ssid;
    int32_t rssi;
    int32_t channel;
};
IfInfo ifInfo();

// ---- SD-based credentials (/wifi.txt, CSV: SSID,passphrase) ----------------
struct Cred { String ssid; String pass; };
int  loadCreds(Cred* out, int maxN);   // returns count loaded (0 if no file/SD)

} // namespace wifi
