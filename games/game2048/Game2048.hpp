#pragma once

#include "Game.hpp"
#include "lvgl.h"
#include <string>
#include <random>

class Game2048 : public Game {
public:
    Game2048();
    ~Game2048() override;

    void run() override;
    void update() override;
    void stop() override;
    void handleKey(uint32_t key) override;
    std::string name() const override { return "2048"; }

private:
    static const int GRID_SIZE = 4;
    static const int CELL_SIZE = 65;
    static const int CELL_SPACING = 6;
    
    void createGameScreen();
    void resetGame();
    void addRandomTile();
    bool move(int dx, int dy);
    bool canMove();
    void updateDisplay();
    void updateScore();
    void gameOver(bool win);
    lv_color_t getTileColor(int value);
    
    lv_obj_t* screen_;
    lv_obj_t* gameBoard_;
    lv_obj_t* scoreLabel_;
    lv_obj_t* tiles_[GRID_SIZE][GRID_SIZE];
    lv_obj_t* tileLabels_[GRID_SIZE][GRID_SIZE];
    
    int grid_[GRID_SIZE][GRID_SIZE];
    int score_;
    bool gameRunning_;
    bool won_;
    bool stopped_ = false;
    
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_int_distribution<> posDist_;
    std::uniform_int_distribution<> valueDist_;
};

struct Register2048 {
    Register2048();
} inline g_register2048;