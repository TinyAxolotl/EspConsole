#pragma once
#include "lvgl.h"
#include <vector>
#include <algorithm>

class TimerManager {
public:
    TimerManager() = default;
    ~TimerManager();

    lv_timer_t* create(uint32_t period_ms, lv_timer_cb_t cb, void* user_data = nullptr);
    void cancel(lv_timer_t* timer);
    void clear();

private:
    std::vector<lv_timer_t*> timers_;
};
