#include "services/Worker.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

bool Worker::start(const char* name, Job job, uint32_t stackWords, int core)
{
    if (_running) return false;
    _job = std::move(job);
    _stop = false;
    _running = true;
    {
        std::lock_guard<std::mutex> lk(_mtx);
        _buf.clear();
    }
    BaseType_t ok = xTaskCreatePinnedToCore(
        trampoline, name, stackWords, this, 1 /*prio*/, nullptr, core);
    if (ok != pdPASS) {
        _running = false;
        return false;
    }
    return true;
}

void Worker::trampoline(void* arg)
{
    Worker* w = static_cast<Worker*>(arg);
    if (w->_job) w->_job(*w);
    w->_running = false;
    vTaskDelete(nullptr);
}

void Worker::emit(const String& line, uint16_t color)
{
    std::lock_guard<std::mutex> lk(_mtx);
    _buf.push_back({line, color});
}

void Worker::drain(const Sink& sink)
{
    std::vector<Line> local;
    {
        std::lock_guard<std::mutex> lk(_mtx);
        if (_buf.empty()) return;
        local.swap(_buf);
    }
    for (auto& l : local) sink(l.text, l.color);
}
