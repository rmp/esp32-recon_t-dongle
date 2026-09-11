// ============================================================================
//  T-Dongle-S3 Diagnostic & Pentest Tool
//
//  A menu-driven, one-button diagnostic/pentest console for the LilyGo
//  T-Dongle-S3. Built with esp_lcd_panel (display) + TFT_eSprite (drawing).
//  USB runs in OTG/TinyUSB composite mode (CDC + HID + MSC).
//
//  Navigation (single BOOT button):
//     click      -> next item
//     hold ~0.6s -> select
//     hold ~2s   -> back / home
//
//  FOR AUTHORIZED SECURITY TESTING ONLY.
// ============================================================================
#include <Arduino.h>
#include "USB.h"

#include "Config.h"
#include "hal/Display.h"
#include "hal/Button.h"
#include "hal/StatusLed.h"

#include "ui/UiManager.h"
#include "ui/MenuScreen.h"
#include "ui/ToolScreen.h"

#include "services/Sd.h"
#include "services/WifiService.h"
#include "services/BleService.h"
#include "services/NetTools.h"
#include "services/HidService.h"
#include "services/MscService.h"
#include "services/SdLog.h"

#include "screens/SystemInfoScreen.h"
#include "screens/WifiScanScreen.h"
#include "screens/UsbStorageScreen.h"
#include "screens/HidScriptsScreen.h"
#include "screens/AboutScreen.h"

#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

// ---- Globals (long-lived singletons; no heap churn) ------------------------
static Display   display;
static Button    button;
static StatusLed led;
static UiManager ui;

static MenuScreen        homeMenu("Diag Tool", "click:next  hold:select");
static MenuScreen        wifiMenu("WiFi");
static MenuScreen        netMenu("Network");
static MenuScreen        bleMenu("Bluetooth");
static MenuScreen        hidMenu("USB HID");

static SystemInfoScreen  sysScreen;
static WifiScanScreen    wifiScan;
static UsbStorageScreen  usbStorage;
static HidScriptsScreen  hidScripts;
static AboutScreen       about;
static ToolScreen        tool;

// ---- Small helpers ---------------------------------------------------------
static void runTool(const char* title, ToolScreen::JobFactory job, const char* banner = "working")
{
    tool.configure(title, std::move(job), banner);
    ui.push(&tool);
}

static bool requireWifi(Worker& w)
{
    if (wifi::isConnected()) return true;
    w.emit("Not connected.", theme::ERR);
    w.emit("Use WiFi > Connect", theme::TEXT_DIM);
    return false;
}

static void jobInterfaceInfo(Worker& w)
{
    if (!requireWifi(w)) return;
    wifi::IfInfo n = wifi::ifInfo();
    w.emit("SSID:  " + n.ssid, theme::ACCENT);
    w.emit("IP:    " + n.ip);
    w.emit("GW:    " + n.gw);
    w.emit("Mask:  " + n.mask);
    w.emit("DNS:   " + n.dns);
    w.emit("MAC:   " + n.mac, theme::TEXT_DIM);
    w.emit("RSSI:  " + String(n.rssi) + " dBm");
    w.emit("Chan:  " + String(n.channel));
}

static bool tryConnect(Worker& w, const char* ssid, const char* pass)
{
    w.emit("Trying " + String(ssid) + " ...", theme::ACCENT);
    wifi::connect(ssid, pass);
    for (int i = 0; i < 30 && !w.stopRequested(); ++i) {
        if (wifi::isConnected()) {
            w.emit("Connected!", theme::OK);
            w.emit("IP " + wifi::ifInfo().ip, theme::OK);
            return true;
        }
        delay(500);
    }
    wifi::disconnect();
    return false;
}

static void jobConnect(Worker& w)
{
    static constexpr int MAX_CREDS = 8;
    wifi::Cred creds[MAX_CREDS];
    int n = wifi::loadCreds(creds, MAX_CREDS);

    if (n > 0) {
        w.emit(String(n) + " network(s) from SD", theme::TEXT_DIM);
        for (int i = 0; i < n && !w.stopRequested(); ++i) {
            if (tryConnect(w, creds[i].ssid.c_str(), creds[i].pass.c_str()))
                return;
            w.emit("No response.", theme::WARN);
        }
    }

    const char* ssid = WIFI_SSID;
    const char* pass = WIFI_PASSWORD;
    if (ssid[0]) {
        if (n > 0) w.emit("Trying built-in...", theme::TEXT_DIM);
        if (tryConnect(w, ssid, pass)) return;
    }

    if (n == 0 && !ssid[0]) {
        w.emit("No wifi.txt on SD", theme::WARN);
        w.emit("No compiled SSID", theme::WARN);
        w.emit("Put wifi.txt on SD:", theme::TEXT_DIM);
        w.emit("  SSID,passphrase", theme::TEXT_DIM);
    } else {
        w.emit("All networks failed.", theme::ERR);
    }
}

// ---- Built-in demo payload (safe, Windows Run box) -------------------------
static const char* DEMO_NOTEPAD =
    "REM Opens Notepad and types a banner (Windows)\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 400\n"
    "STRING notepad\n"
    "ENTER\n"
    "DELAY 800\n"
    "STRING T-Dongle-S3 HID test. Authorized use only.\n";

static void buildMenus()
{
    homeMenu.addItem("System Info", [] { ui.push(&sysScreen); });
    homeMenu.addItem("WiFi",        [] { ui.push(&wifiMenu); });
    homeMenu.addItem("Bluetooth",   [] { ui.push(&bleMenu); });
    homeMenu.addItem("Network",     [] { ui.push(&netMenu); });
    homeMenu.addItem("USB HID",     [] { ui.push(&hidMenu); });
    homeMenu.addItem("USB Storage", [] { ui.push(&usbStorage); });
    homeMenu.addItem("About",       [] { ui.push(&about); });

    wifiMenu.addItem("Scan APs",       [] { ui.push(&wifiScan); });
    wifiMenu.addItem("Interface info", [] { runTool("WiFi iface", jobInterfaceInfo); });
    wifiMenu.addItem("Connect", [] { runTool("WiFi connect", jobConnect, "connecting"); });
    wifiMenu.addItem("Disconnect",     [] {
        runTool("WiFi", [](Worker& w) { wifi::disconnect(); w.emit("Disconnected.", theme::WARN); });
    });

    wifiScan.setOnSelect([](const wifi::Ap& a) {
        wifi::Ap ap = a;
        runTool("AP detail", [ap](Worker& w) {
            w.emit("SSID:  " + ap.ssid, theme::ACCENT);
            w.emit("BSSID: " + ap.bssid, theme::TEXT_DIM);
            w.emit("RSSI:  " + String(ap.rssi) + " dBm");
            w.emit("Chan:  " + String(ap.channel));
            w.emit("Enc:   " + String(wifi::encStr(ap.enc)));
            w.emit(ap.hidden ? "Hidden network" : "");
        });
    });

    bleMenu.addItem("Scan 6s",  [] { runTool("BLE scan", [](Worker& w) { ble::scan(w, 6); }, "scanning"); });
    bleMenu.addItem("Scan 15s", [] { runTool("BLE scan", [](Worker& w) { ble::scan(w, 15); }, "scanning"); });

    netMenu.addItem("Interface info", [] { runTool("WiFi iface", jobInterfaceInfo); });
    netMenu.addItem("Ping gateway", [] {
        runTool("Ping GW", [](Worker& w) {
            if (!requireWifi(w)) return;
            nettools::pingHost(w, WiFi.gatewayIP(), 4);
        }, "pinging");
    });
    netMenu.addItem("Ping 1.1.1.1", [] {
        runTool("Ping 1.1.1.1", [](Worker& w) {
            if (!requireWifi(w)) return;
            nettools::pingHost(w, IPAddress(1, 1, 1, 1), 4);
        }, "pinging");
    });
    netMenu.addItem("Portscan gateway", [] {
        runTool("Portscan GW", [](Worker& w) {
            if (!requireWifi(w)) return;
            nettools::portScan(w, WiFi.gatewayIP(),
                               nettools::COMMON_PORTS, nettools::COMMON_PORTS_N);
        }, "scanning");
    });

    hidScripts.setOnRun([](const String& path) {
        runTool("Payload", [path](Worker& w) { hid::runDuckyFile(w, path); }, "typing");
    });
    hidMenu.addItem("Payloads (SD)", [] { ui.push(&hidScripts); });
    hidMenu.addItem("Demo: type", [] {
        runTool("HID type", [](Worker& w) {
            hid::typeString(w, "Hello from T-Dongle-S3\n");
        }, "typing");
    });
    hidMenu.addItem("Demo: Win+R notepad", [] {
        runTool("HID demo", [](Worker& w) { hid::runDuckyText(w, DEMO_NOTEPAD); }, "typing");
    });
    hidMenu.addItem("Mouse jiggler 60s", [] {
        runTool("Mouse jiggle", [](Worker& w) { hid::mouseJiggle(w, 60); }, "jiggling");
    });
}

void setup()
{
    delay(500);
    Serial.begin(115200);

    led.begin();
    led.set(LedState::Busy);

    if (!display.begin()) {
        while (true) { led.set(LedState::Error); delay(500); led.set(LedState::Off); delay(500); }
    }

    button.begin(PIN_BTN);

    sd::begin();
    sdlog::begin();
    sdlog::section("BOOT");
    sdlog::log("sys", FW_NAME " " FW_VERSION " started");

    hid::begin();
    msc::begin();
    USB.begin();

    hidScripts.setDir("/payloads");
    buildMenus();

    ui.begin(&display, &button, &led);
    ui.push(&homeMenu);

    led.set(LedState::Idle);
}

void loop()
{
    ui.loop();
    delay(2);
}
