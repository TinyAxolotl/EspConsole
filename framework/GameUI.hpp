#pragma once
#include "lvgl.h"

class GameUI {
public:
    class Screen {
    public:
        Screen() = default;
        explicit Screen(lv_obj_t* obj);
        Screen(const Screen&) = delete;
        Screen& operator=(const Screen&) = delete;
        Screen(Screen&& other) noexcept;
        Screen& operator=(Screen&& other) noexcept;
        ~Screen();

        lv_obj_t* get() const;
        void load();
    private:
        void cleanup();
        lv_obj_t* obj_ = nullptr;
    };

    Screen createScreen();
    lv_obj_t* createLabel(lv_obj_t* parent, const char* text,
                          lv_align_t align = LV_ALIGN_TOP_LEFT,
                          lv_coord_t x_off = 0, lv_coord_t y_off = 0) const;

    void applyCleanStyle(lv_obj_t* obj) const;
};
