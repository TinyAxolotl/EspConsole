#pragma once

#include "Screen.hpp"
#include "GameRegistry.hpp"
#include "InputRouter.hpp"
#include "lvgl.h"
#include <cstddef>
#include <cstdlib>
#include <functional>

class MenuScreen : public Screen {
public:
    using GameSelectedCallback = std::function<void(const GameFactory&)>;

    MenuScreen();
    ~MenuScreen() override = default;

    // Screen interface
    void show() override;
    void hide() override;
    
    // Input handling
    void handleInput(uint32_t key);
    
    // Set callback for game selection
    void setGameSelectedCallback(GameSelectedCallback callback);

private:
    void createUI();
    void updateSelection();
    
    lv_obj_t* screen_ = nullptr;
    lv_obj_t* list_ = nullptr;
    lv_obj_t* title_ = nullptr;
    lv_obj_t* items_[10] = {nullptr};
    
    int selectedIndex_ = 0;
    GameSelectedCallback gameSelectedCallback_ = nullptr;
    const std::vector<GameFactory>& games_;
};
