#include "screens/HidScriptsScreen.h"
#include "ui/Theme.h"
#include "Config.h"
#include "services/HidService.h"
#include "services/Sd.h"

int HidScriptsScreen::visibleRows() const
{
    int h = theme::contentHeight(SCREEN_H);
    int rows = h / theme::ROW_H;
    return rows < 1 ? 1 : rows;
}

void HidScriptsScreen::ensureVisible()
{
    int rows = visibleRows();
    if (_sel < _top) _top = _sel;
    else if (_sel >= _top + rows) _top = _sel - rows + 1;
    if (_top < 0) _top = 0;
}

void HidScriptsScreen::onEnter()
{
    _files = sd::mounted() ? hid::listScripts(_dir) : std::vector<String>();
    _sel = 0;
    _top = 0;
    markDirty();
}

bool HidScriptsScreen::handle(InputEvent e)
{
    if (_files.empty()) return false;   // any Back exits
    switch (e) {
        case InputEvent::Next:
            _sel = (_sel + 1) % (int)_files.size();
            ensureVisible();
            markDirty();
            return true;
        case InputEvent::Select:
            if (_onRun) _onRun(String(_dir) + "/" + _files[_sel]);
            return true;
        default:
            return false;
    }
}

void HidScriptsScreen::draw(Display& d)
{
    TFT_eSprite& g = d.canvas();
    theme::drawHeader(g, "Payloads",
                      _files.empty() ? -1 : _sel,
                      _files.empty() ? -1 : (int)_files.size());

    int16_t y = theme::contentTop();
    g.setTextFont(1);
    g.setTextDatum(TL_DATUM);

    if (_files.empty()) {
        g.setTextColor(theme::WARN, theme::BG);
        g.drawString(sd::mounted() ? "No payloads found" : "No SD card", theme::PAD, y + 2);
        g.setTextColor(theme::TEXT_DIM, theme::BG);
        g.drawString("Add .txt/.dd to", theme::PAD, y + 2 + theme::ROW_H);
        g.drawString(_dir, theme::PAD, y + 2 + 2 * theme::ROW_H);
        theme::drawFooter(g, "vlong:back");
        return;
    }

    int rows = visibleRows();
    for (int i = 0; i < rows; ++i) {
        int idx = _top + i;
        if (idx >= (int)_files.size()) break;
        theme::drawListRow(g, y, _files[idx].c_str(), idx == _sel);
        y += theme::ROW_H;
    }

    theme::drawFooter(g, "click:next  hold:RUN");
}
