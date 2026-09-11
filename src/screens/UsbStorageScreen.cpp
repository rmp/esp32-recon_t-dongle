#include "screens/UsbStorageScreen.h"
#include "ui/Theme.h"
#include "ui/UiManager.h"
#include "Config.h"
#include "services/Sd.h"
#include "services/MscService.h"

void UsbStorageScreen::onEnter()
{
    markDirty();
}

void UsbStorageScreen::onExit()
{
    // Always hide the card again on leaving, to avoid firmware/host races.
    if (msc::exposed()) msc::expose(false);
    if (_ui && _ui->led()) _ui->led()->set(LedState::Idle);
}

bool UsbStorageScreen::handle(InputEvent e)
{
    if (e == InputEvent::Select) {
        if (msc::available()) {
            bool now = !msc::exposed();
            msc::expose(now);
            if (_ui && _ui->led())
                _ui->led()->set(now ? LedState::Active : LedState::Idle);
        }
        markDirty();
        return true;
    }
    return false;   // Back exits (onExit hides media)
}

void UsbStorageScreen::draw(Display& d)
{
    TFT_eSprite& g = d.canvas();
    theme::drawHeader(g, "USB Storage");

    int16_t y = theme::contentTop();
    g.setTextFont(1);
    g.setTextDatum(TL_DATUM);

    if (!sd::mounted()) {
        g.setTextColor(theme::ERR, theme::BG);
        g.drawString("No SD card detected.", theme::PAD, y + 2);
        g.setTextColor(theme::TEXT_DIM, theme::BG);
        g.drawString("Insert card + reboot.", theme::PAD, y + 2 + theme::ROW_H);
        theme::drawFooter(g, "vlong:back");
        return;
    }

    String card = String(sd::typeStr()) + " " + sd::humanSize(sd::sizeBytes());
    g.setTextColor(theme::TEXT, theme::BG);
    g.drawString(card, theme::PAD, y + 2);
    y += theme::ROW_H;

    bool on = msc::exposed();
    g.setTextColor(theme::TEXT_DIM, theme::BG);
    g.drawString("Host drive:", theme::PAD, y + 2);
    g.setTextDatum(TR_DATUM);
    g.setTextColor(on ? theme::OK : theme::WARN, theme::BG);
    g.drawString(on ? "EXPOSED" : "hidden", SCREEN_W - theme::PAD, y + 2);
    g.setTextDatum(TL_DATUM);
    y += theme::ROW_H;

    g.setTextColor(theme::TEXT_DIM, theme::BG);
    g.drawString(on ? "Card is mounted on host" : "hold: expose to host", theme::PAD, y + 2);

    theme::drawFooter(g, on ? "hold:hide  vlong:back" : "hold:expose  vlong:back");
}
