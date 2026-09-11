// ============================================================================
//  HidService.h  -  USB HID keyboard + mouse, and a DuckyScript interpreter
//  (BadUSB) for authorized security testing.
//
//  The HID interfaces are registered once at boot (see main setup) so the
//  composite USB device enumerates deterministically. Payloads run inside a
//  Worker job and stream progress back to the UI; they can be stopped mid-run.
// ============================================================================
#pragma once

#include <Arduino.h>
#include <vector>
#include "services/Worker.h"

namespace hid {

// Register HID keyboard + mouse with the USB stack. Call once, before USB.begin().
void begin();
bool ready();

// Payload execution (run inside a Worker job).
void runDuckyText(Worker& w, const String& text);
void runDuckyFile(Worker& w, const String& path);   // read a script from SD
void typeString(Worker& w, const String& s);
void mouseJiggle(Worker& w, uint32_t seconds);       // keep-awake / anti-idle

// Enumerate *.txt / *.dd DuckyScript files in a directory on the SD card.
std::vector<String> listScripts(const String& dir = "/payloads");

} // namespace hid
