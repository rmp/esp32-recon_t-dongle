#include "screens/SystemInfoScreen.h"
#include "ui/Theme.h"
#include "Config.h"

int SystemInfoScreen::visibleRows() const
{
    int h = theme::contentHeight(SCREEN_H);
    int rows = h / theme::ROW_H;
    return rows < 1 ? 1 : rows;
}

int SystemInfoScreen::maxTop() const
{
    int rows = visibleRows();
    int n = (int)_rows.size();
    return (n > rows) ? (n - rows) : 0;
}

void SystemInfoScreen::refresh()
{
    _rows = sysinfo::snapshot();
    if (_top > maxTop()) _top = maxTop();
    markDirty();
}

void SystemInfoScreen::onEnter()
{
    _top = 0;
    _lastRefresh = millis();
    refresh();
}

void SystemInfoScreen::update()
{
    if (millis() - _lastRefresh >= 1000) {
        _lastRefresh = millis();
        refresh();
    }
}

bool SystemInfoScreen::handle(InputEvent e)
{
    switch (e) {
        case InputEvent::Next:
            _top = (_top >= maxTop()) ? 0 : min(_top + (visibleRows() - 1), maxTop());
            markDirty();
            return true;
        case InputEvent::Select:
            _top = 0;
            markDirty();
            return true;
        default:
            return false;   // Back exits
    }
}

void SystemInfoScreen::draw(Display& d)
{
    TFT_eSprite& g = d.canvas();
    theme::drawHeader(g, "System Info", _top, (int)_rows.size());

    int rows = visibleRows();
    int16_t y = theme::contentTop();
    for (int i = 0; i < rows; ++i) {
        int idx = _top + i;
        if (idx >= (int)_rows.size()) break;
        g.setTextFont(1);
        g.setTextDatum(TL_DATUM);
        g.setTextColor(theme::TEXT_DIM, theme::BG);
        g.drawString(_rows[idx].key, theme::PAD, y + 2);
        g.setTextDatum(TR_DATUM);
        g.setTextColor(theme::TEXT, theme::BG);
        g.drawString(_rows[idx].value, SCREEN_W - theme::PAD, y + 2);
        y += theme::ROW_H;
    }

    theme::drawFooter(g, "click:scroll  vlong:back");
}
