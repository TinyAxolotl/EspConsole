#pragma once
#include <functional>
#include <cstdint>
#include "lvgl.h"

class InputRouter {
public:
    using Callback = std::function<void(uint32_t)>;
    static InputRouter* instance();
    void setCallback(Callback cb);
    void dispatchKey(uint32_t key);
    lv_indev_t* indev() const;
private:
    InputRouter();
    Callback cb_;
    lv_indev_t* indev_ = nullptr;
};
