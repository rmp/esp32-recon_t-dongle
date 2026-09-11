#include "screens/WifiScanScreen.h"
#include "ui/Theme.h"
#include "Config.h"
#include "services/SdLog.h"

int WifiScanScreen::visibleRows() const
{
    int h = theme::contentHeight(SCREEN_H);
    int rows = h / theme::ROW_H;
    return rows < 1 ? 1 : rows;
}

void WifiScanScreen::ensureVisible()
{
    int rows = visibleRows();
    if (_sel < _top) _top = _sel;
    else if (_sel >= _top + rows) _top = _sel - rows + 1;
    if (_top < 0) _top = 0;
}

void WifiScanScreen::startScan()
{
    _aps.clear();
    _scanning = true;
    _sel = 0;
    _top = 0;
    wifi::startScan(true);
    markDirty();
}

void WifiScanScreen::onEnter() { startScan(); }

void WifiScanScreen::update()
{
    if (!_scanning) return;
    int st = wifi::scanState();
    if (st >= 0) {
        int n = st;
        _aps.clear();
        _aps.reserve(n);
        for (int i = 0; i < n; ++i) _aps.push_back(wifi::ap(i));
        _scanning = false;
        markDirty();
        sdlog::section("WiFi Scan");
        for (int i = 0; i < n; ++i) {
            const wifi::Ap& a = _aps[i];
            sdlog::log("wifi", a.ssid + "  " + a.bssid + "  ch" +
                       String(a.channel) + "  " + String(a.rssi) + "dBm  " +
                       wifi::encStr(a.enc));
        }
        sdlog::log("wifi", String(n) + " AP(s) found");
    } else if (st == WIFI_SCAN_FAILED) {
        _scanning = false;
        markDirty();
    }
}

bool WifiScanScreen::handle(InputEvent e)
{
    if (_scanning) {
        if (e == InputEvent::Back) return false;
        return true;                 // ignore nav while scanning
    }
    switch (e) {
        case InputEvent::Next:
            _sel = (_sel + 1) % itemCount();
            ensureVisible();
            markDirty();
            return true;
        case InputEvent::Select:
            if (_sel == 0) { startScan(); }
            else if (_onSelect && _sel - 1 < (int)_aps.size()) { _onSelect(_aps[_sel - 1]); }
            return true;
        case InputEvent::Back:
        default:
            return false;
    }
}

void WifiScanScreen::draw(Display& d)
{
    TFT_eSprite& g = d.canvas();
    theme::drawHeader(g, "WiFi APs", _scanning ? -1 : _sel, _scanning ? -1 : itemCount());

    int16_t y = theme::contentTop();

    if (_scanning) {
        g.setTextFont(1);
        g.setTextColor(theme::WARN, theme::BG);
        g.setTextDatum(TL_DATUM);
        static const char* dots[] = {"", ".", "..", "..."};
        String s = String("Scanning") + dots[(millis() / 300) & 3];
        g.drawString(s, theme::PAD, y + 2);
        theme::drawFooter(g, "vlong:back");
        return;
    }

    int rows = visibleRows();
    for (int i = 0; i < rows; ++i) {
        int idx = _top + i;
        if (idx >= itemCount()) break;
        bool selected = (idx == _sel);
        if (idx == 0) {
            theme::drawListRow(g, y, "Rescan", selected, nullptr, theme::ACCENT);
        } else {
            const wifi::Ap& a = _aps[idx - 1];
            char right[20];
            snprintf(right, sizeof(right), "%s %ld", wifi::encStr(a.enc), (long)a.rssi);
            String name = a.ssid;
            if (name.length() > 16) name = name.substring(0, 16);
            theme::drawListRow(g, y, name.c_str(), selected, right);
        }
        y += theme::ROW_H;
    }

    theme::drawFooter(g, "click:next  hold:open");
}
