#include "InputRouter.hpp"
#include "lvgl.h"
#include "app.h"
#include <cstdio>
#include "esp_log.h"

static const char *TAG = "InputRouter";

extern "C" void (*handle_input_event)(uint32_t key);

InputRouter* InputRouter::instance() {
  static InputRouter inst;
  return &inst;
}

InputRouter::InputRouter() {
    // Capture indev from lvgl_init
    indev_ = lv_indev_get_next(nullptr);
    while (indev_ && lv_indev_get_type(indev_) != LV_INDEV_TYPE_KEYPAD) {
        indev_ = lv_indev_get_next(indev_);
    }

    // Set global handler
    handle_input_event = [](uint32_t k) {
        ESP_LOGI(TAG, "Key press detected in InputRouter: %lu", k);
        InputRouter::instance()->dispatchKey(k);
    };
    
    ESP_LOGI(TAG, "InputRouter initialized");
}

void InputRouter::dispatchKey(uint32_t key) {
    ESP_LOGI(TAG, "InputRouter dispatching key: %lu", key);
    if (cb_) {
        cb_(key);
    } else {
        ESP_LOGW(TAG, "No callback registered in InputRouter");
    }
}

void InputRouter::setCallback(Callback cb) {
    ESP_LOGI(TAG, "InputRouter callback set");
    cb_ = std::move(cb);
}

lv_indev_t* InputRouter::indev() const {
    return indev_;
}