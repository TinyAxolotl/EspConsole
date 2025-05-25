#pragma once
#include "lvgl.h"

class Screen {
public:
    virtual ~Screen() = default;
    virtual void show() = 0;
    virtual void hide() = 0;
};
