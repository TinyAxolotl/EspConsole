#include "MenuScreen.hpp"
#include "esp_log.h"
#include "lvgl.h"
#include "lvgl_helper.hpp"
#include <cstdio>

#define TAG "MenuScreen"

MenuScreen::MenuScreen() 
    : games_(GameRegistry::instance().available())
{
    createUI();
}

void MenuScreen::createUI() {
    screen_ = createCleanObject(nullptr);
    lv_obj_set_style_bg_color(screen_, lv_color_make(255, 255, 255), 0);
    title_ = lv_label_create(screen_);
    applyCleanStyle(title_);
    lv_label_set_text(title_, "Game Console");
    lv_obj_align(title_, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_text_font(title_, &lv_font_montserrat_24, 0);

    list_ = lv_obj_create(screen_);
    applyCleanStyle(list_);
    lv_obj_set_style_bg_color(list_, lv_color_make(255, 255, 255), 0);
    lv_obj_set_style_bg_opa(list_, LV_OPA_COVER, 0);

    lv_obj_add_flag(list_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(list_, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(list_, LV_SCROLLBAR_MODE_AUTO);

    lv_obj_set_layout(list_, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(list_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(list_, 8, 0);

    lv_obj_set_style_pad_left(list_, 20, 0);
    lv_obj_set_style_pad_right(list_, 20, 0);

    lv_obj_set_size(list_, 280, 300);
    lv_obj_align(list_, LV_ALIGN_CENTER, 0, 20);

    ESP_LOGI(TAG, "Creating menu UI, games count: %zu\n", games_.size());

    if (games_.empty()) {
        auto *lbl = lv_label_create(list_);
        applyCleanStyle(lbl);
        lv_label_set_text(lbl, "No games available.\nCheck registration.");
        lv_obj_set_style_bg_opa(lbl, LV_OPA_COVER, 0);
        lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);
    }
    else {
        for (size_t i = 0; i < games_.size() && i < 10; ++i) {
            items_[i] = lv_btn_create(list_);
            applyCleanStyle(items_[i]);
            lv_obj_set_style_bg_color(items_[i], lv_color_make(200, 200, 200), 0);
            lv_obj_set_style_bg_opa(items_[i], LV_OPA_COVER, 0);

            lv_obj_set_width(items_[i], lv_pct(100));
            lv_obj_set_height(items_[i], 50);

            auto *lbl = lv_label_create(items_[i]);
            applyCleanStyle(lbl);
            lv_label_set_text(lbl, games_[i].name.c_str());
            lv_obj_center(lbl);
        }
        updateSelection();
    }

    hint_ = lv_label_create(screen_);
    applyCleanStyle(hint_);
    lv_label_set_text(hint_, "UP/DOWN: Navigate\nENTER: Select");
    lv_obj_align(hint_, LV_ALIGN_BOTTOM_MID, 0, -20);
}



void MenuScreen::show() {
    lv_scr_load(screen_);
}

void MenuScreen::hide() {
}

void MenuScreen::handleInput(uint32_t key) {
    if (games_.empty()) return;

    bool changed = false;
    switch (key) {
        case LV_KEY_UP:
            if (selectedIndex_ > 0) { selectedIndex_--; changed = true; }
            ESP_LOGI(TAG, "Menu UP pressed, current index: %d\n", selectedIndex_);
            break;
        case LV_KEY_DOWN:
            if (selectedIndex_ < static_cast<int>(games_.size()) - 1 && selectedIndex_ < 9) {
                selectedIndex_++; changed = true;
            }
            ESP_LOGI(TAG, "Menu DOWN pressed, current index: %d\n", selectedIndex_);
            break;
        case LV_KEY_ENTER:
            if (gameSelectedCallback_) {
                ESP_LOGI(TAG, "Menu ENTER pressed at index: %d\n", selectedIndex_);
                gameSelectedCallback_(games_[selectedIndex_]);
            }
            break;
        default:
            break;
    }
    if (changed) {
        updateSelection();
    }
}

void MenuScreen::updateSelection() {
    lv_obj_scroll_to_view(items_[selectedIndex_], LV_ANIM_ON);

    for (int i = 0; i < static_cast<int>(games_.size()) && i < 10; ++i) {
        if (i == selectedIndex_) {
            lv_obj_set_style_bg_color(items_[i], lv_color_make(255, 128, 0), 0);
        } else {
            lv_obj_set_style_bg_color(items_[i], lv_color_make(32, 32, 32), 0);
        }
    }
}

void MenuScreen::setGameSelectedCallback(GameSelectedCallback cb) {
    gameSelectedCallback_ = std::move(cb);
}