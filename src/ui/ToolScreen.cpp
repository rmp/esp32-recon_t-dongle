#include "ui/ToolScreen.h"
#include "ui/UiManager.h"
#include "services/SdLog.h"

void ToolScreen::configure(const char* title, JobFactory job, const char* banner)
{
    setTitle(title);
    _job = std::move(job);
    _banner = banner ? banner : "working";
}

void ToolScreen::onEnter()
{
    resetLines();
    markDirty();
    if (!_job) return;
    sdlog::section(title());
    setBusy(true, _banner);
    _wasRunning = true;
    if (_ui && _ui->led()) _ui->led()->set(LedState::Busy);
    _worker.start("tool", [this](Worker& w) { _job(w); });
}

void ToolScreen::waitStop(uint32_t timeoutMs)
{
    _worker.requestStop();
    uint32_t end = millis() + timeoutMs;
    while (_worker.running() && (int32_t)(end - millis()) > 0) {
        delay(5);
    }
}

void ToolScreen::onExit()
{
    // Signal the job to stop and give it a moment so the instance can be reused.
    waitStop(1500);
    if (_ui && _ui->led()) _ui->led()->set(LedState::Idle);
}

void ToolScreen::update()
{
    _worker.drain([this](const String& s, uint16_t c) {
        addLine(s, c);
        sdlog::log(title(), s);
    });
    if (!_worker.running() && _wasRunning) {
        _wasRunning = false;
        setBusy(false);
        if (_ui && _ui->led()) _ui->led()->set(LedState::Ok);
    }
}

bool ToolScreen::handle(InputEvent e)
{
    if (e == InputEvent::Back) {
        _worker.requestStop();      // stop the job; UiManager will pop us
        return false;
    }
    return ScrollScreen::handle(e);
}
