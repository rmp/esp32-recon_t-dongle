// ============================================================================
//  Worker.h  -  Run a job on a background FreeRTOS task and stream text lines
//  back to the UI thread. Long operations (port scans, ping sweeps, BLE scans)
//  stay off the render loop so the single button remains responsive.
//
//  Usage:
//    worker.start("scan", [](Worker& w){
//        for (...) { if (w.stopRequested()) break; w.emit("line", color); }
//    });
//    // in Screen::update():  worker.drain([&](const String& s, uint16_t c){ addLine(s,c); });
//    //                       if (!worker.running()) setBusy(false);
// ============================================================================
#pragma once

#include <Arduino.h>
#include <functional>
#include <vector>
#include <mutex>

class Worker {
public:
    using Job  = std::function<void(Worker&)>;
    using Sink = std::function<void(const String&, uint16_t)>;

    bool start(const char* name, Job job, uint32_t stackWords = 6144, int core = 1);

    // Called from the job (background task).
    void emit(const String& line, uint16_t color = 0xFFFF);
    bool stopRequested() const { return _stop; }

    // Called from the UI thread.
    void drain(const Sink& sink);
    bool running() const { return _running; }
    void requestStop() { _stop = true; }

private:
    struct Line { String text; uint16_t color; };

    static void trampoline(void* arg);

    Job  _job;
    std::vector<Line> _buf;
    std::mutex _mtx;
    volatile bool _running = false;
    volatile bool _stop = false;
};
