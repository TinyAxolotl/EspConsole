#include "lvgl_helper.hpp"

lv_obj_t* createCleanObject(lv_obj_t* parent) {
    lv_obj_t* obj = lv_obj_create(parent);
    applyCleanStyle(obj);
    //lv_obj_remove_style_all(obj);
    return obj;
}

void applyCleanStyle(lv_obj_t* obj) {
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_layout(obj, LV_LAYOUT_NONE);
}

lv_obj_t* createLabel(lv_obj_t* parent, const char* text, lv_align_t align, lv_coord_t x_off, lv_coord_t y_off) {
    lv_obj_t* label = lv_label_create(parent);
    applyCleanStyle(label);
    lv_label_set_text(label, text);
    lv_obj_align(label, align, x_off, y_off);
    return label;
}
