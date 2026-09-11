// ============================================================================
//  SysInfo.h  -  Read-only host/device facts for the System Info screen and
//  for the header of other tools (chip, memory, flash, MACs, uptime, temp).
// ============================================================================
#pragma once

#include <Arduino.h>
#include <vector>

namespace sysinfo {

struct KV { String key; String value; };

// A snapshot of device facts, ready to render as rows.
std::vector<KV> snapshot();

String uptimeStr();
float  temperatureC();

} // namespace sysinfo
