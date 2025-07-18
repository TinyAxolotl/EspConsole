#pragma once
#include "GameUI.hpp"
#include "TimerManager.hpp"
#include "InputRouter.hpp"

struct GameContext {
    GameUI& ui;
    TimerManager& timers;
    InputRouter& input;
    lv_display_t* display;

    GameContext(GameUI& ui_, TimerManager& timers_, InputRouter& input_, lv_display_t* disp = nullptr)
        : ui(ui_), timers(timers_), input(input_), display(disp) {}
};
