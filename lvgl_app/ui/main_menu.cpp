#include "main_menu.hpp"
#include "lvgl.h"

void initialize_menu(bool *menu_initialized) {
    lv_obj_clean(lv_scr_act());

    lv_obj_t* title_label = lv_label_create(lv_scr_act());
    lv_label_set_text(title_label, "Select Game:");
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 20);

    lv_obj_t* btn_template = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_template, 180, 50);
    lv_obj_align(btn_template, LV_ALIGN_TOP_MID, 0, 60);

    lv_obj_t* label_snake = lv_label_create(btn_template);
    lv_label_set_text(label_snake, "Template #1");
    lv_obj_center(label_snake);

    *menu_initialized = true;
}