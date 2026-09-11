// ============================================================================
//  MscService.h  -  Expose the microSD card to the host as USB mass storage.
//
//  The MSC interface is registered once at boot (with the card's geometry) so
//  enumeration is deterministic; the media is toggled present/absent at run
//  time. While media is exposed, firmware must not touch the filesystem.
// ============================================================================
#pragma once

#include <Arduino.h>

namespace msc {

void begin();          // register interface if an SD card is mounted
bool available();      // card present and interface registered
void expose(bool on);  // present/remove the media to the host
bool exposed();

} // namespace msc
