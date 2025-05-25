#include "InputRouter.hpp"
#include "lvgl.h"
#include "app.h"
#include <cstdio>

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
        printf("Key press detected in InputRouter: %lu\n", k);
        InputRouter::instance()->dispatchKey(k);
    };
    
    printf("InputRouter initialized\n");
}

void InputRouter::dispatchKey(uint32_t key) {
    printf("InputRouter dispatching key: %lu\n", key);
    if (cb_) {
        cb_(key);
    } else {
        printf("WARNING: No callback registered in InputRouter\n");
    }
}

void InputRouter::setCallback(Callback cb) {
    printf("InputRouter callback set\n");
    cb_ = std::move(cb);
}

lv_indev_t* InputRouter::indev() const {
    return indev_;
}