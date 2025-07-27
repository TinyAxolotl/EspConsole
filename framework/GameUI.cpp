#include "GameUI.hpp"

GameUI::Screen::Screen(lv_obj_t* obj) : obj_(obj) {}

GameUI::Screen::Screen(Screen&& other) noexcept : obj_(other.obj_) {
    other.obj_ = nullptr;
}

GameUI::Screen& GameUI::Screen::operator=(Screen&& other) noexcept {
    if (this != &other) {
        cleanup();
        obj_ = other.obj_;
        other.obj_ = nullptr;
    }
    return *this;
}

GameUI::Screen::~Screen() {
    cleanup();
}

lv_obj_t* GameUI::Screen::get() const {
    return obj_;
}

void GameUI::Screen::load() {
    if (obj_) {
        lv_scr_load(obj_);
    }
}

void GameUI::Screen::cleanup() {
    if (obj_ && lv_obj_is_valid(obj_)) {
        lv_obj_del(obj_);
        obj_ = nullptr;
    }
}

GameUI::Screen GameUI::createScreen() {
    lv_obj_t* scr = lv_obj_create(nullptr);
    applyCleanStyle(scr);
    return Screen(scr);
}

lv_obj_t* GameUI::createLabel(lv_obj_t* parent, const char* text,
                              lv_align_t align, lv_coord_t x_off, lv_coord_t y_off) const {
    lv_obj_t* label = lv_label_create(parent);
    applyCleanStyle(label);
    if (text) {
        lv_label_set_text(label, text);
    }
    lv_obj_align(label, align, x_off, y_off);
    return label;
}

lv_obj_t* GameUI::createRect(lv_obj_t* parent, lv_coord_t w, lv_coord_t h, lv_color_t color) {
    lv_obj_t* obj = lv_obj_create(parent);
    applyCleanStyle(obj);
    setSize(obj, w, h);
    setBgColor(obj, color);
    return obj;
}

void GameUI::setSize(lv_obj_t* obj, lv_coord_t w, lv_coord_t h) const { lv_obj_set_size(obj, w, h); }
void GameUI::setPos(lv_obj_t* obj, lv_coord_t x, lv_coord_t y) const { lv_obj_set_pos(obj, x, y); }
void GameUI::setX(lv_obj_t* obj, lv_coord_t x) const { lv_obj_set_x(obj, x); }
void GameUI::setBgColor(lv_obj_t* obj, lv_color_t color) const { lv_obj_set_style_bg_color(obj, color, 0); }
void GameUI::setBorder(lv_obj_t* obj, lv_coord_t width, lv_color_t color) const {
    lv_obj_set_style_border_width(obj, width, 0);
    lv_obj_set_style_border_color(obj, color, 0);
}
void GameUI::setRadius(lv_obj_t* obj, lv_coord_t radius) const { lv_obj_set_style_radius(obj, radius, 0); }
void GameUI::setBgOpacity(lv_obj_t* obj, lv_opa_t opa) const { lv_obj_set_style_bg_opa(obj, opa, 0); }
void GameUI::setTextColor(lv_obj_t* obj, lv_color_t color) const { lv_obj_set_style_text_color(obj, color, 0); }
void GameUI::setTextFont(lv_obj_t* obj, const lv_font_t* font) const { lv_obj_set_style_text_font(obj, font, 0); }
void GameUI::setTextAlign(lv_obj_t* obj, lv_text_align_t align) const { lv_obj_set_style_text_align(obj, align, 0); }
void GameUI::setLabelText(lv_obj_t* label, const char* text) const { lv_label_set_text(label, text); }
void GameUI::align(lv_obj_t* obj, lv_align_t align, lv_coord_t x_off, lv_coord_t y_off) const { lv_obj_align(obj, align, x_off, y_off); }
void GameUI::center(lv_obj_t* obj) const { lv_obj_center(obj); }
void GameUI::remove(lv_obj_t* obj) const { if(obj) lv_obj_del(obj); }

lv_color_t GameUI::color(uint8_t r, uint8_t g, uint8_t b) const { return lv_color_make(r, g, b); }
uint32_t GameUI::tick() const { return lv_tick_get(); }

void GameUI::applyCleanStyle(lv_obj_t* obj) const {
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_layout(obj, LV_LAYOUT_NONE);
}
