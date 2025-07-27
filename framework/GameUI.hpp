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

    lv_obj_t* createRect(lv_obj_t* parent, lv_coord_t w, lv_coord_t h, lv_color_t color);

    void setSize(lv_obj_t* obj, lv_coord_t w, lv_coord_t h) const;
    void setPos(lv_obj_t* obj, lv_coord_t x, lv_coord_t y) const;
    void setX(lv_obj_t* obj, lv_coord_t x) const;
    void setBgColor(lv_obj_t* obj, lv_color_t color) const;
    void setBorder(lv_obj_t* obj, lv_coord_t width, lv_color_t color) const;
    void setRadius(lv_obj_t* obj, lv_coord_t radius) const;
    void setBgOpacity(lv_obj_t* obj, lv_opa_t opa) const;
    void setTextColor(lv_obj_t* obj, lv_color_t color) const;
    void setTextFont(lv_obj_t* obj, const lv_font_t* font) const;
    void setTextAlign(lv_obj_t* obj, lv_text_align_t align) const;
    void setLabelText(lv_obj_t* label, const char* text) const;
    void align(lv_obj_t* obj, lv_align_t align, lv_coord_t x_off = 0, lv_coord_t y_off = 0) const;
    void center(lv_obj_t* obj) const;
    void remove(lv_obj_t* obj) const;

    lv_color_t color(uint8_t r, uint8_t g, uint8_t b) const;
    uint32_t tick() const;

    void applyCleanStyle(lv_obj_t* obj) const;
};
