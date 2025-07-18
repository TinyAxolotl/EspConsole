#pragma once

#include "Game.hpp"
#include "lvgl.h"
#include <string>
#include <vector>
#include <random>
#include <queue>

class Minesweeper : public Game {
public:
    Minesweeper();
    ~Minesweeper() override;

    void run() override;
    void update() override;
    void stop() override;
    void handleKey(uint32_t key) override;
    std::string name() const override { return "Minesweeper"; }
    void processRevealStep();

private:
    static const int GRID_WIDTH = 9;
    static const int GRID_HEIGHT = 9;
    static const int CELL_SIZE = 30;
    static const int MINE_COUNT = 10;
    
    enum CellState {
        HIDDEN,
        REVEALED,
        FLAGGED
    };
    
    struct Cell {
        lv_obj_t* obj;
        lv_obj_t* label;
        bool hasMine;
        int adjacentMines;
        CellState state;
    };
    
    void createGameScreen();
    void resetGame();
    void placeMines();
    void calculateNumbers();
    void revealCell(int x, int y);
    void toggleFlag(int x, int y);
    void revealAllMines();
    void checkWin();
    void updateDisplay();
    void gameOver(bool win);
    void startRevealFrom(int x, int y);
    lv_color_t getNumberColor(int number);
    
    lv_obj_t* screen_;
    lv_obj_t* gameBoard_;
    lv_obj_t* statusLabel_;
    lv_obj_t* mineCountLabel_;
    lv_obj_t* cursor_;
    
    Cell grid_[GRID_HEIGHT][GRID_WIDTH];

    std::queue<std::pair<int, int>> revealQueue_;
    lv_timer_t* revealTimer_ = nullptr;
    int cursorX_;
    int cursorY_;
    int flagCount_;
    int revealedCount_;
    bool gameRunning_;
    bool firstClick_;
    int totalFlags_;
    bool stopped_ = false;
    std::random_device rd_;
    std::mt19937 gen_;
};

struct RegisterMinesweeper {
    RegisterMinesweeper();
} inline g_registerMinesweeper;