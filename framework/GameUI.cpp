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

void GameUI::applyCleanStyle(lv_obj_t* obj) const {
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_layout(obj, LV_LAYOUT_NONE);
}
