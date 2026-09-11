#include "services/SdLog.h"
#include "services/Sd.h"
#include <SD_MMC.h>
#include <mutex>

namespace sdlog {

static bool s_ready = false;
static std::mutex s_mtx;

void begin()
{
    if (!sd::mounted()) return;
    if (!SD_MMC.exists("/logs")) SD_MMC.mkdir("/logs");
    s_ready = true;
}

static String timestamp()
{
    unsigned long s = millis() / 1000;
    unsigned h = s / 3600; s %= 3600;
    unsigned m = s / 60;   s %= 60;
    char buf[16];
    snprintf(buf, sizeof(buf), "%02u:%02u:%02u", h, m, (unsigned)s);
    return String(buf);
}

static String logPath()
{
    unsigned long boot = millis() / 1000;
    char buf[32];
    snprintf(buf, sizeof(buf), "/logs/session_%lu.log", boot < 10 ? 0UL : boot);
    return String(buf);
}

static String s_path;

static void ensurePath()
{
    if (s_path.length() == 0) s_path = logPath();
}

void log(const char* tag, const String& msg)
{
    if (!s_ready) return;
    std::lock_guard<std::mutex> lk(s_mtx);
    ensurePath();
    File f = SD_MMC.open(s_path.c_str(), FILE_APPEND);
    if (!f) return;
    f.print(timestamp());
    f.print(" [");
    f.print(tag);
    f.print("] ");
    f.println(msg);
    f.close();
}

void section(const char* tag)
{
    if (!s_ready) return;
    std::lock_guard<std::mutex> lk(s_mtx);
    ensurePath();
    File f = SD_MMC.open(s_path.c_str(), FILE_APPEND);
    if (!f) return;
    f.println();
    f.print("=== ");
    f.print(timestamp());
    f.print(" ");
    f.print(tag);
    f.println(" ===");
    f.close();
}

} // namespace sdlog
