#include "ui/MenuScreen.h"
#include "ui/Theme.h"
#include "Config.h"

int MenuScreen::visibleRows() const
{
    int h = theme::contentHeight(SCREEN_H);
    int rows = h / theme::ROW_H;
    return rows < 1 ? 1 : rows;
}

void MenuScreen::ensureVisible()
{
    int rows = visibleRows();
    if (_sel < _top)            _top = _sel;
    else if (_sel >= _top + rows) _top = _sel - rows + 1;
    if (_top < 0) _top = 0;
}

bool MenuScreen::handle(InputEvent e)
{
    if (_items.empty()) return false;

    switch (e) {
        case InputEvent::Next:
            _sel = (_sel + 1) % (int)_items.size();
            ensureVisible();
            markDirty();
            return true;
        case InputEvent::Select:
            if (_items[_sel].cb) _items[_sel].cb();
            markDirty();
            return true;
        case InputEvent::Back:
        default:
            return false;   // let UiManager pop to parent
    }
}

void MenuScreen::draw(Display& d)
{
    TFT_eSprite& g = d.canvas();
    theme::drawHeader(g, _title, _sel, (int)_items.size());

    const int rows = visibleRows();
    const int16_t x0 = 0;
    int16_t y = theme::contentTop();

    for (int i = 0; i < rows; ++i) {
        int idx = _top + i;
        if (idx >= (int)_items.size()) break;

        bool selected = (idx == _sel);
        if (selected) {
            g.fillRect(x0, y, SCREEN_W, theme::ROW_H, theme::SEL_BG);
            g.setTextColor(theme::SEL_FG, theme::SEL_BG);
        } else {
            g.setTextColor(theme::TEXT, theme::BG);
        }
        g.setTextFont(1);
        g.setTextDatum(TL_DATUM);
        g.drawString(_items[idx].label, theme::PAD + (selected ? 2 : 0), y + 2);

        // Scroll indicators.
        if (i == 0 && _top > 0)
            g.drawString("^", SCREEN_W - 8, y + 2);
        if (i == rows - 1 && (_top + rows) < (int)_items.size())
            g.drawString("v", SCREEN_W - 8, y + 2);

        y += theme::ROW_H;
    }

    theme::drawFooter(g, _footer);
}
