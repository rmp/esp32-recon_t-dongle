// ============================================================================
//  BleService.h  -  Passive/active BLE discovery. Runs a timed scan inside a
//  Worker job and streams discovered devices (address, RSSI, name, service)
//  back to the UI.
// ============================================================================
#pragma once

#include <Arduino.h>
#include "services/Worker.h"

namespace ble {

void scan(Worker& w, uint32_t seconds = 6);

} // namespace ble
