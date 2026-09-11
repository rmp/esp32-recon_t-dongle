#include "services/Sd.h"
#include "Config.h"

// Access the protected _card member of SDMMCFS for raw sector I/O.
struct SdCardAccessor : fs::SDMMCFS {
    sdmmc_card_t* getCard() { return _card; }
};

namespace sd {

static bool s_mounted = false;

bool begin()
{
    if (s_mounted) return true;
    SD_MMC.setPins(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0, PIN_SD_D1, PIN_SD_D2, PIN_SD_D3);
    s_mounted = SD_MMC.begin("/sdcard", false);
    return s_mounted;
}

sdmmc_card_t* card()
{
    if (!mounted()) return nullptr;
    return reinterpret_cast<SdCardAccessor*>(&SD_MMC)->getCard();
}

bool mounted() { return s_mounted && SD_MMC.cardType() != CARD_NONE; }

uint64_t sizeBytes() { return mounted() ? SD_MMC.cardSize() : 0; }
uint64_t usedBytes() { return mounted() ? SD_MMC.usedBytes() : 0; }

const char* typeStr()
{
    if (!s_mounted) return "NONE";
    switch (SD_MMC.cardType()) {
        case CARD_MMC:  return "MMC";
        case CARD_SD:   return "SDSC";
        case CARD_SDHC: return "SDHC";
        default:        return "NONE";
    }
}

uint32_t sectorCount()
{
    sdmmc_card_t* c = card();
    return c ? c->csd.capacity : 0;
}

uint16_t sectorSize()
{
    sdmmc_card_t* c = card();
    return c ? c->csd.sector_size : 0;
}

String humanSize(uint64_t bytes)
{
    const char* u[] = {"B", "KB", "MB", "GB", "TB"};
    double v = (double)bytes;
    int i = 0;
    while (v >= 1024.0 && i < 4) { v /= 1024.0; i++; }
    char buf[24];
    snprintf(buf, sizeof(buf), (v < 10 && i > 0) ? "%.1f%s" : "%.0f%s", v, u[i]);
    return String(buf);
}

} // namespace sd
