#pragma once

#include "lvgl.h"

// Create LVGL-obj w/o scrolls, paddings & layouts
lv_obj_t* createCleanObject(lv_obj_t* parent);

// Quick lable generation with predefined text (With styles autocleanup)
lv_obj_t* createLabel(lv_obj_t* parent, const char* text, lv_align_t align = LV_ALIGN_TOP_LEFT, lv_coord_t x_off = 0, lv_coord_t y_off = 0);

// Setup zero-padding, disable scroll & layout — apply on already exsiting object
void applyCleanStyle(lv_obj_t* obj);
