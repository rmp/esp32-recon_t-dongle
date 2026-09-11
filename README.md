# T-Dongle-S3 Diagnostic & Pentest Tool

A menu-driven diagnostic and security-testing console for the
**LilyGo T-Dongle-S3** (ESP32-S3). Intended for authorised penetration
testing and network troubleshooting.

## Hardware

| Component | Detail |
|---|---|
| MCU | ESP32-S3 (QFN56, dual-core 240 MHz, WiFi + BLE 5) |
| Flash | 16 MB QSPI |
| SRAM | 512 KB (no PSRAM) |
| Display | ST7735 TFT, 160 × 80 px, SPI |
| Status LED | APA102 (single RGB, SPI-clocked) |
| Input | One button — BOOT / GPIO 0 |
| Storage | microSD slot (SD_MMC, 4-bit) |
| USB | Native USB-OTG via TinyUSB (CDC + HID + MSC composite) |
| Connector | USB-A male (plugs directly into a host) |

### Pin Map

```
Display (ST7735, SPI)          SD Card (SD_MMC, 4-bit)
  MOSI   GPIO 3                  CLK    GPIO 12
  SCLK   GPIO 5                  CMD    GPIO 16
  CS     GPIO 4                  D0     GPIO 14
  DC     GPIO 2                  D1     GPIO 17
  RST    GPIO 1                  D2     GPIO 21
  BL     GPIO 38 (active LOW)    D3     GPIO 18

APA102 LED                     Misc
  DI     GPIO 40                 BOOT   GPIO 0  (user button)
  CI     GPIO 39                 TX     GPIO 43 (QWIIC)
                                 RX     GPIO 44 (QWIIC)
```

> **Note:** GPIO 39 and GPIO 40 are JTAG strapping pins (MTCK / MTDO).
> When the device is in ROM download mode (firmware not running), the JTAG
> controller drives these pins and the APA102 shows solid white. This is
> normal — it means the firmware has not booted yet.

### Post-Flash Boot Requirement

After flashing via USB-Serial/JTAG, the esptool "Hard resetting via RTS
pin" **does not** cold-boot the firmware. You must **unplug and replug**
the dongle (without holding BOOT) after every flash for the firmware to
start.

## Features

- **WiFi recon** — async AP scan with SSID, BSSID, channel, RSSI,
  encryption; results logged to SD
- **BLE recon** — active BLE scan (6 s / 15 s) showing address, RSSI,
  name, service UUID
- **Network diagnostics** — ICMP ping, TCP port scan (26 common ports),
  interface info
- **USB HID (BadUSB)** — DuckyScript interpreter; load `.txt` payloads
  from `/payloads/` on the SD card, or run built-in demos (type string,
  Win+R notepad, mouse jiggler)
- **USB mass storage** — expose the microSD to the host as a removable
  drive
- **System info** — chip, MAC addresses, free heap, flash size, uptime,
  temperature
- **SD logging** — all scan results, tool output, and actions are
  timestamped and written to `/logs/` on the microSD (when present)
- **WiFi credentials from SD** — place a `wifi.txt` file on the SD root
  (`SSID,passphrase` per line) to connect without recompiling

### One-Button Navigation

The T-Dongle-S3 has a single physical button. All navigation uses
duration-based gestures:

| Gesture | Duration | Action |
|---|---|---|
| Short click | < 600 ms | Next item / scroll |
| Long press | 600 – 2000 ms | Select / enter |
| Very-long press | > 2000 ms | Back / home |

A progress bar overlays the footer while the button is held, showing
whether the current hold will trigger Select or Back.

## Software Architecture

```
src/
├── main.cpp              Application entry: setup, menu wiring, tool jobs
├── Config.h              Pin map, gesture timing, constants
│
├── hal/                  Hardware Abstraction Layer
│   ├── Display.h/cpp       ST7735 via esp_lcd_panel + TFT_eSprite framebuffer
│   ├── Button.h/cpp        One-button gesture decoder (Next/Select/Back)
│   └── StatusLed.h/cpp     APA102 LED with semantic states (Idle/Busy/Ok/Error)
│
├── ui/                   UI Framework
│   ├── Screen.h            Base class — onEnter/onExit/update/handle/draw
│   ├── UiManager.h/cpp     Screen stack, input routing, frame pump (~30 fps)
│   ├── MenuScreen.h/cpp    Generic scrollable menu with callbacks
│   ├── ScrollScreen.h/cpp  Read-only scrollable line output (tool results)
│   ├── ToolScreen.h/cpp    Runs a Worker job and streams output to ScrollScreen
│   └── Theme.h/cpp         Shared palette (RGB565), layout metrics, draw helpers
│
├── screens/              Concrete Screens
│   ├── SystemInfoScreen    Chip / memory / uptime facts
│   ├── WifiScanScreen      Async AP scan with selectable result list
│   ├── HidScriptsScreen    Browse and run DuckyScript payloads from SD
│   ├── UsbStorageScreen    Toggle SD card as USB mass storage
│   └── AboutScreen         Firmware version and credits
│
├── services/             Backend Logic (no UI dependency)
│   ├── WifiService         Async scan, connect, interface info, SD credential loading
│   ├── BleService          BLE active scan via NimBLE
│   ├── NetTools            ICMP ping (esp_ping), TCP port scan
│   ├── HidService          USB HID keyboard/mouse, DuckyScript interpreter
│   ├── MscService          USB mass storage (raw sector I/O on SD)
│   ├── Sd                  SD_MMC mount and helpers
│   ├── SdLog               Timestamped session logging to /logs/ on SD
│   ├── SysInfo             Device fact snapshot
│   └── Worker              FreeRTOS background task with thread-safe line output
│
└── bsp_lcd/              Board Support — ST7735 panel driver for esp_lcd API
    └── esp_lcd_st7735.h/c  (from LilyGo T-Dongle-S3 examples)
```

### Key Design Decisions

**Display driver** — The display is driven by the ESP-IDF `esp_lcd_panel`
API (not TFT_eSPI). `TFT_eSprite` is still used as a pure in-memory
16-bit framebuffer for drawing; `Display::flush()` pushes the sprite's
raw pixel buffer to the panel via `esp_lcd_panel_draw_bitmap()`.

**Worker pattern** — Long-running operations (scans, pings, HID
payloads) execute on a dedicated FreeRTOS task. They emit coloured text
lines through a thread-safe queue that the UI drains each frame.
The ToolScreen both displays and logs these lines.

**USB composite** — USB runs in OTG/TinyUSB mode (`ARDUINO_USB_MODE=0`)
to support a CDC + HID + MSC composite device. The HID and MSC
interfaces are dynamically allocated (not static globals) to avoid
`tinyusb_enable_interface()` calls during static initialisation.

**Single-instance screens** — All screens are long-lived singletons
allocated at file scope, not on the heap. The `UiManager` holds a stack
of `Screen*` pointers (max depth 8). This avoids heap fragmentation on a
device with 512 KB SRAM and no PSRAM.

## Build

Requires [PlatformIO](https://platformio.org/).

```sh
# Build
pio run

# Flash (then unplug/replug the dongle)
pio run -t upload

# Serial monitor
pio device monitor -b 115200
```

### Build Environments

| Environment | Board | USB Mode | Purpose |
|---|---|---|---|
| `tdongle-s3` (default) | `tdongle_s3_otg` | TinyUSB (MODE=0) | Production firmware |
| `diag` | `dongles3` | HWCDC (MODE=1) | Minimal LCD + LED diagnostic |

### SD Card Layout

```
/
├── wifi.txt              WiFi credentials (SSID,passphrase per line)
├── payloads/             DuckyScript .txt files for BadUSB
│   ├── example_hello.txt
│   └── win_ipconfig.txt
└── logs/                 Auto-created session logs
    └── session_*.log
```

## License

See LICENCE.txt
