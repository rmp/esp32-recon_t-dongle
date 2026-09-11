#include "services/MscService.h"
#include "services/Sd.h"
#include "USB.h"
#include "USBMSC.h"
#include "sdmmc_cmd.h"

namespace msc {

static USBMSC* s_msc = nullptr;
static bool s_available = false;
static bool s_exposed   = false;

static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
{
    (void)offset;
    sdmmc_card_t* c = sd::card();
    if (!c) return -1;
    uint32_t secSize = sd::sectorSize();
    if (!secSize) return -1;
    uint32_t count = bufsize / secSize;
    if (sdmmc_write_sectors(c, buffer, lba, count) != ESP_OK) return -1;
    return bufsize;
}

static int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
{
    (void)offset;
    sdmmc_card_t* c = sd::card();
    if (!c) return -1;
    uint32_t secSize = sd::sectorSize();
    if (!secSize) return -1;
    uint32_t count = bufsize / secSize;
    if (sdmmc_read_sectors(c, buffer, lba, count) != ESP_OK) return -1;
    return bufsize;
}

static bool onStartStop(uint8_t power_condition, bool start, bool load_eject)
{
    (void)power_condition; (void)start; (void)load_eject;
    return true;
}

void begin()
{
    if (!sd::mounted()) { s_available = false; return; }

    s_msc = new USBMSC();
    s_msc->vendorID("LilyGo");
    s_msc->productID("TDongle-SD");
    s_msc->productRevision("1.0");
    s_msc->onStartStop(onStartStop);
    s_msc->onRead(onRead);
    s_msc->onWrite(onWrite);
    s_msc->mediaPresent(false);
    s_msc->begin(sd::sectorCount(), sd::sectorSize());
    s_available = true;
    s_exposed = false;
}

bool available() { return s_available; }
bool exposed()   { return s_exposed; }

void expose(bool on)
{
    if (!s_available || !s_msc) return;
    s_msc->mediaPresent(on);
    s_exposed = on;
}

} // namespace msc
