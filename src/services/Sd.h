// ============================================================================
//  Sd.h  -  microSD (SD_MMC, 4-bit) mount + small helpers.
//
//  The card is mounted once at boot. File-based tools (e.g. DuckyScript) use
//  the normal FS API; the USB mass-storage service uses raw sector IO on the
//  same mounted card. Do not run a file tool and USB-MSC at the same time.
// ============================================================================
#pragma once

#include <Arduino.h>
#include <SD_MMC.h>
#include "driver/sdmmc_types.h"

namespace sd {

bool     begin();                 // mount; safe to call repeatedly
bool     mounted();
uint64_t sizeBytes();
uint64_t usedBytes();
const char* typeStr();            // "SDHC", "SDSC", "MMC", "NONE"
uint32_t sectorCount();
uint16_t sectorSize();
sdmmc_card_t* card();             // underlying card handle for raw sector I/O
String   humanSize(uint64_t bytes);

} // namespace sd
