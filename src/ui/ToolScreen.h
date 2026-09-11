// ============================================================================
//  ToolScreen.h  -  Runs a background job and streams its output.
//
//  A single shared instance backs every "run a thing and show lines" tool
//  (ping, port scan, BLE scan, HID payloads, mouse jiggler, info dumps). Only
//  one tool is active at a time (single screen stack + single button), so one
//  instance reconfigured per use is safe and avoids per-tool boilerplate.
// ============================================================================
#pragma once

#include <functional>
#include "ui/ScrollScreen.h"
#include "services/Worker.h"

class ToolScreen : public ScrollScreen {
public:
    using JobFactory = std::function<void(Worker&)>;

    ToolScreen() : ScrollScreen("Tool") {}

    // Configure just before pushing. `banner` is shown while the job runs.
    void configure(const char* title, JobFactory job, const char* banner = "working");

    void onEnter() override;
    void onExit()  override;
    void update()  override;
    bool handle(InputEvent e) override;

private:
    Worker      _worker;
    JobFactory  _job;
    const char* _banner = "working";
    bool        _wasRunning = false;
    void waitStop(uint32_t timeoutMs);
};
