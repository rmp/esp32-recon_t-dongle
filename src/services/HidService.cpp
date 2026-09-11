#include "services/HidService.h"
#include "ui/Theme.h"
#include <FS.h>
#include <SD_MMC.h>
#include "USB.h"
#include "USBHIDKeyboard.h"
#include "USBHIDMouse.h"

using fs::File;

namespace hid {

static USBHIDKeyboard* s_kbd   = nullptr;
static USBHIDMouse*    s_mouse = nullptr;
static bool s_ready = false;

void begin()
{
    if (s_ready) return;
    s_kbd = new USBHIDKeyboard();
    s_mouse = new USBHIDMouse();
    s_kbd->begin();
    s_mouse->begin();
    s_ready = true;
}

bool ready() { return s_ready; }

// ---- interruptible delay ---------------------------------------------------
static void idleDelay(Worker& w, uint32_t ms)
{
    uint32_t end = millis() + ms;
    while ((int32_t)(end - millis()) > 0) {
        if (w.stopRequested()) return;
        uint32_t chunk = end - millis();
        delay(chunk > 20 ? 20 : chunk);
    }
}

// ---- DuckyScript key-name -> keycode --------------------------------------
struct KeyName { const char* name; uint8_t code; };
static const KeyName KEYS[] = {
    {"ENTER", KEY_RETURN}, {"RETURN", KEY_RETURN},
    {"ESC", KEY_ESC}, {"ESCAPE", KEY_ESC},
    {"TAB", KEY_TAB}, {"SPACE", ' '},
    {"BACKSPACE", KEY_BACKSPACE}, {"DEL", KEY_DELETE}, {"DELETE", KEY_DELETE},
    {"INSERT", KEY_INSERT}, {"HOME", KEY_HOME}, {"END", KEY_END},
    {"PAGEUP", KEY_PAGE_UP}, {"PAGEDOWN", KEY_PAGE_DOWN},
    {"UP", KEY_UP_ARROW}, {"UPARROW", KEY_UP_ARROW},
    {"DOWN", KEY_DOWN_ARROW}, {"DOWNARROW", KEY_DOWN_ARROW},
    {"LEFT", KEY_LEFT_ARROW}, {"LEFTARROW", KEY_LEFT_ARROW},
    {"RIGHT", KEY_RIGHT_ARROW}, {"RIGHTARROW", KEY_RIGHT_ARROW},
    {"CAPSLOCK", KEY_CAPS_LOCK},
    {"GUI", KEY_LEFT_GUI}, {"WINDOWS", KEY_LEFT_GUI}, {"WIN", KEY_LEFT_GUI},
    {"CTRL", KEY_LEFT_CTRL}, {"CONTROL", KEY_LEFT_CTRL},
    {"ALT", KEY_LEFT_ALT}, {"SHIFT", KEY_LEFT_SHIFT},
    {"F1", KEY_F1}, {"F2", KEY_F2}, {"F3", KEY_F3}, {"F4", KEY_F4},
    {"F5", KEY_F5}, {"F6", KEY_F6}, {"F7", KEY_F7}, {"F8", KEY_F8},
    {"F9", KEY_F9}, {"F10", KEY_F10}, {"F11", KEY_F11}, {"F12", KEY_F12},
};

static bool keyFromName(const String& up, uint8_t& out)
{
    for (auto& k : KEYS) if (up == k.name) { out = k.code; return true; }
    return false;
}

// Press a whitespace-separated combo like "CTRL ALT DELETE" or "GUI r".
static void pressCombo(const String& line)
{
    int start = 0;
    int pressed = 0;
    while (start < (int)line.length() && pressed < 6) {
        int sp = line.indexOf(' ', start);
        String tok = (sp < 0) ? line.substring(start) : line.substring(start, sp);
        tok.trim();
        if (tok.length()) {
            String up = tok; up.toUpperCase();
            uint8_t code;
            if (keyFromName(up, code)) {
                s_kbd->press(code);
            } else if (tok.length() == 1) {
                s_kbd->press((uint8_t)tok[0]);
            }
            pressed++;
        }
        if (sp < 0) break;
        start = sp + 1;
    }
    delay(8);
    s_kbd->releaseAll();
}

static bool startsWithCmd(const String& up, const char* cmd) { return up.startsWith(cmd); }

void runDuckyText(Worker& w, const String& text)
{
    if (!s_ready) { w.emit("HID not ready", theme::ERR); return; }

    uint32_t defaultDelay = 0;
    String prevLine;
    int lineNo = 0;
    int start = 0;
    const int len = text.length();

    w.emit("Running payload...", theme::DANGER);

    while (start <= len && !w.stopRequested()) {
        int nl = text.indexOf('\n', start);
        String raw = (nl < 0) ? text.substring(start) : text.substring(start, nl);
        start = (nl < 0) ? len + 1 : nl + 1;
        lineNo++;

        // Normalize line endings / trailing CR.
        raw.replace("\r", "");
        String line = raw;
        line.trim();
        if (line.length() == 0) { continue; }

        String up = line; up.toUpperCase();

        if (startsWithCmd(up, "REM")) {
            // comment
        } else if (startsWithCmd(up, "DEFAULT_DELAY") || startsWithCmd(up, "DEFAULTDELAY")) {
            int sp = line.indexOf(' ');
            defaultDelay = (sp > 0) ? (uint32_t)line.substring(sp + 1).toInt() : 0;
        } else if (startsWithCmd(up, "DELAY")) {
            int sp = line.indexOf(' ');
            uint32_t ms = (sp > 0) ? (uint32_t)line.substring(sp + 1).toInt() : 0;
            idleDelay(w, ms);
        } else if (startsWithCmd(up, "STRINGLN")) {
            String s = (line.length() > 8) ? line.substring(9) : "";
            s_kbd->print(s);
            s_kbd->write(KEY_RETURN);
        } else if (startsWithCmd(up, "STRING")) {
            String s = (line.length() > 6) ? line.substring(7) : "";
            s_kbd->print(s);
        } else if (startsWithCmd(up, "REPEAT")) {
            int sp = line.indexOf(' ');
            int n = (sp > 0) ? line.substring(sp + 1).toInt() : 0;
            for (int i = 0; i < n && !w.stopRequested(); ++i) {
                if (prevLine.length()) { pressCombo(prevLine); idleDelay(w, defaultDelay); }
            }
        } else {
            // Treat the whole line as a key / combo.
            pressCombo(line);
        }

        if (!startsWithCmd(up, "REM")) prevLine = line;
        if (lineNo % 4 == 0) w.emit("  ." + String(lineNo) + " " + line.substring(0, 18), theme::TEXT_DIM);
        idleDelay(w, defaultDelay);
    }

    if (w.stopRequested()) w.emit("-- stopped --", theme::WARN);
    else                   w.emit("Payload complete.", theme::OK);
}

void runDuckyFile(Worker& w, const String& path)
{
    File f = SD_MMC.open(path, "r");
    if (!f) { w.emit("open failed: " + path, theme::ERR); return; }
    String text = f.readString();
    f.close();
    w.emit("Loaded " + path + " (" + String(text.length()) + "B)", theme::ACCENT);
    runDuckyText(w, text);
}

void typeString(Worker& w, const String& s)
{
    if (!s_ready) { w.emit("HID not ready", theme::ERR); return; }
    w.emit("Typing " + String(s.length()) + " chars", theme::DANGER);
    s_kbd->print(s);
    w.emit("Done.", theme::OK);
}

void mouseJiggle(Worker& w, uint32_t seconds)
{
    if (!s_ready) { w.emit("HID not ready", theme::ERR); return; }
    w.emit("Mouse jiggle " + String(seconds) + "s", theme::DANGER);
    uint32_t end = millis() + seconds * 1000UL;
    int dir = 1;
    while ((int32_t)(end - millis()) > 0 && !w.stopRequested()) {
        s_mouse->move(3 * dir, 0);
        delay(120);
        s_mouse->move(-3 * dir, 0);
        dir = -dir;
        idleDelay(w, 800);
    }
    w.emit(w.stopRequested() ? "-- stopped --" : "Done.",
           w.stopRequested() ? theme::WARN : theme::OK);
}

std::vector<String> listScripts(const String& dir)
{
    std::vector<String> out;
    File root = SD_MMC.open(dir);
    if (!root || !root.isDirectory()) return out;
    File f = root.openNextFile();
    while (f) {
        if (!f.isDirectory()) {
            String name = f.name();
            String lower = name; lower.toLowerCase();
            if (lower.endsWith(".txt") || lower.endsWith(".dd")) {
                // f.name() may be a full path depending on core version.
                int slash = name.lastIndexOf('/');
                out.push_back(slash >= 0 ? name.substring(slash + 1) : name);
            }
        }
        f = root.openNextFile();
    }
    root.close();
    return out;
}

} // namespace hid
