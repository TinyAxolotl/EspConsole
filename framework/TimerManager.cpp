#include "TimerManager.hpp"

lv_timer_t* TimerManager::create(uint32_t period_ms, lv_timer_cb_t cb, void* user_data) {
    lv_timer_t* t = lv_timer_create(cb, period_ms, user_data);
    if (t) {
        timers_.push_back(t);
    }
    return t;
}

void TimerManager::cancel(lv_timer_t* timer) {
    if (!timer) return;
    auto it = std::find(timers_.begin(), timers_.end(), timer);
    if (it != timers_.end()) {
        lv_timer_del(timer);
        timers_.erase(it);
    }
}

void TimerManager::clear() {
    for (auto* t : timers_) {
        lv_timer_del(t);
    }
    timers_.clear();
}

TimerManager::~TimerManager() {
    clear();
}
