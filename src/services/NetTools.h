// ============================================================================
//  NetTools.h  -  Host-side network diagnostics for a connected STA:
//  ICMP ping, TCP port scanning, and hostname resolution. Each long routine
//  runs inside a Worker job and streams result lines back to the UI.
// ============================================================================
#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "services/Worker.h"

namespace nettools {

extern const uint16_t COMMON_PORTS[];
extern const size_t    COMMON_PORTS_N;
const char* portName(uint16_t port);

bool resolve(const String& host, IPAddress& out);

// Run inside a Worker job. Emit lines as they happen; honour w.stopRequested().
void portScan(Worker& w, IPAddress host, const uint16_t* ports, size_t n,
              uint32_t timeoutMs = 400);
void pingHost(Worker& w, IPAddress host, int count = 4);

} // namespace nettools
