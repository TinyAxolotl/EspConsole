#include "MenuScreen.hpp"
#include "lvgl.h"
#include <cstdio>

MenuScreen::MenuScreen() 
    : games_(GameRegistry::instance().available())
{
    printf("MenuScreen constructor, available games: %zu\n", games_.size());
    createUI();
}

void MenuScreen::createUI() {
    screen_ = lv_obj_create(nullptr);
    
    title_ = lv_label_create(screen_);
    lv_label_set_text(title_, "Game Console");
    lv_obj_align(title_, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_text_font(title_, &lv_font_montserrat_24, 0);
    
    list_ = lv_obj_create(screen_);
    lv_obj_set_size(list_, 280, 300);
    lv_obj_set_style_bg_opa(list_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(list_, 0, 0);
    lv_obj_align(list_, LV_ALIGN_CENTER, 0, 20);
    
    const auto& games = GameRegistry::instance().available();
    printf("Creating menu UI, games count: %zu\n", games.size());
    
    if (games.empty()) {
        lv_obj_t* noGamesLabel = lv_label_create(list_);
        lv_label_set_text(noGamesLabel, "No games available.\nCheck registration.");
        lv_obj_align(noGamesLabel, LV_ALIGN_CENTER, 0, 0);
        printf("Warning: No games to display in menu!\n");
    } else {
        for (size_t i = 0; i < games.size() && i < 10; i++) {
            printf("Adding game to menu: %s\n", games[i].name.c_str());
            items_[i] = lv_btn_create(list_);
            lv_obj_set_size(items_[i], 260, 50);
            lv_obj_set_pos(items_[i], 10, i * 60);
            
            lv_obj_t* label = lv_label_create(items_[i]);
            lv_label_set_text(label, games[i].name.c_str());
            lv_obj_center(label);
        }
        
        if (!games.empty()) {
            updateSelection();
        }
    }
    
    if (!games_.empty()) {
        updateSelection();
    }
    
    lv_obj_t* hint = lv_label_create(screen_);
    lv_label_set_text(hint, "UP/DOWN: Navigate\nENTER: Select");
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -20);
}

void MenuScreen::show() {
    lv_scr_load(screen_);
}

void MenuScreen::hide() {
}

void MenuScreen::handleInput(uint32_t key) {
    printf("MenuScreen handling key: %lu\n", key);
    printf("Key values: UP=%d, DOWN=%d, LEFT=%d, RIGHT=%d, ENTER=%d\n", 
       LV_KEY_UP, LV_KEY_DOWN, LV_KEY_LEFT, LV_KEY_RIGHT, LV_KEY_ENTER);
    if (games_.empty()) {
        printf("No games available in menu\n");
        return;
    }
    
    bool selectionChanged = false;
    
    switch (key) {
        case LV_KEY_UP:
            printf("Menu UP pressed, current index: %d\n", selectedIndex_);
            if (selectedIndex_ > 0) {
                selectedIndex_--;
                selectionChanged = true;
                printf("Selection changed to %d\n", selectedIndex_);
            }
            break;
            
        case LV_KEY_DOWN:
            printf("Menu DOWN pressed, current index: %d\n", selectedIndex_);
            if (selectedIndex_ < static_cast<int>(games_.size()) - 1 && selectedIndex_ < 9) {
                selectedIndex_++;
                selectionChanged = true;
                printf("Selection changed to %d\n", selectedIndex_);
            }
            break;
            
        case LV_KEY_ENTER:
            printf("Menu ENTER pressed at index: %d\n", selectedIndex_);
            if (gameSelectedCallback_ && selectedIndex_ >= 0 && 
                selectedIndex_ < static_cast<int>(games_.size())) {
                printf("Calling game selection callback\n");
                gameSelectedCallback_(games_[selectedIndex_]);
            } else {
                printf("No callback or invalid selection\n");
            }
            break;
            
        default:
            printf("Unhandled key in menu: %lu\n", key);
            break;
    }
    
    if (selectionChanged) {
        printf("Updating selection in menu\n");
        updateSelection();
    }
}

void MenuScreen::updateSelection() {
    for (size_t i = 0; i < games_.size() && i < 10; i++) {
        if (i == static_cast<size_t>(selectedIndex_)) {
            lv_obj_set_style_bg_color(items_[i], lv_color_make(0, 128, 255), 0);
        } else {
            lv_obj_set_style_bg_color(items_[i], lv_color_make(32, 32, 32), 0);
        }
    }
}

void MenuScreen::setGameSelectedCallback(GameSelectedCallback callback) {
    gameSelectedCallback_ = std::move(callback);
}