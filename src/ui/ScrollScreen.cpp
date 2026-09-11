#include "ui/ScrollScreen.h"
#include "Config.h"

int ScrollScreen::visibleRows() const
{
    int h = theme::contentHeight(SCREEN_H);
    int rows = h / theme::ROW_H;
    return rows < 1 ? 1 : rows;
}

int ScrollScreen::maxTop() const
{
    int rows = visibleRows();
    int n = (int)_lines.size();
    return (n > rows) ? (n - rows) : 0;
}

void ScrollScreen::resetLines()
{
    _lines.clear();
    _top = 0;
    _follow = true;
    markDirty();
}

void ScrollScreen::addLine(const String& s, uint16_t color)
{
    if (_lines.size() >= MAX_LINES) {
        _lines.erase(_lines.begin());
        if (_top > 0) _top--;
    }
    _lines.push_back({s, color});
    if (_follow) _top = maxTop();
    markDirty();
}

void ScrollScreen::setBusy(bool busy, const char* banner)
{
    _busy = busy;
    _banner = banner;
    markDirty();
}

bool ScrollScreen::handle(InputEvent e)
{
    switch (e) {
        case InputEvent::Next: {
            int rows = visibleRows();
            int step = rows > 1 ? rows - 1 : 1;   // page down, keep 1 line of context
            if (_top >= maxTop()) {
                _top = 0;                          // already at bottom -> wrap to top
            } else {
                _top = min(_top + step, maxTop());
            }
            _follow = (_top >= maxTop());
            markDirty();
            return true;
        }
        case InputEvent::Select:
            // Toggle follow / jump to bottom.
            _top = maxTop();
            _follow = true;
            markDirty();
            return true;
        case InputEvent::Back:
        default:
            return false;   // exit
    }
}

const char* ScrollScreen::footerHint() const
{
    return "click:scroll  vlong:back";
}

void ScrollScreen::draw(Display& d)
{
    TFT_eSprite& g = d.canvas();

    int total = (int)_lines.size();
    int rows = visibleRows();
    int shown = min(rows, total - _top);
    // Header shows scroll position as first-visible/total.
    theme::drawHeader(g, _title,
                      total ? _top : -1,
                      total ? total : -1);

    int16_t y = theme::contentTop();
    for (int i = 0; i < shown; ++i) {
        int idx = _top + i;
        if (idx < 0 || idx >= total) break;
        g.setTextColor(_lines[idx].color, theme::BG);
        g.setTextFont(1);
        g.setTextDatum(TL_DATUM);
        g.drawString(_lines[idx].text, theme::PAD, y + 2);
        y += theme::ROW_H;
    }

    if (_busy) {
        // Busy banner across the footer.
        const int16_t fy = SCREEN_H - theme::FOOTER_H;
        g.fillRect(0, fy, SCREEN_W, theme::FOOTER_H, theme::HEADER_BG);
        g.setTextColor(theme::WARN, theme::HEADER_BG);
        g.setTextDatum(TL_DATUM);
        g.setTextFont(1);
        // simple animated dots
        static const char* dots[] = {"", ".", "..", "..."};
        uint8_t p = (millis() / 300) & 3;
        String b = String(_banner ? _banner : "working") + dots[p];
        g.drawString(b, theme::PAD, fy + 2);
    } else {
        theme::drawFooter(g, footerHint());
    }
}
