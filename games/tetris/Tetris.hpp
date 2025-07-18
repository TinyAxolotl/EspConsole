#pragma once

#include "Game.hpp"
#include "lvgl.h"
#include <string>
#include <vector>
#include <random>

class Tetris : public Game {
public:
    Tetris();
    ~Tetris() override;

    void run() override;
    void update() override;
    void stop() override;
    void handleKey(uint32_t key) override;
    std::string name() const override { return "Tetris"; }

private:
    static const int BOARD_WIDTH = 10;
    static const int BOARD_HEIGHT = 20;
    static const int CELL_SIZE = 15;
    
    enum TetrominoType {
        I_PIECE = 0,
        O_PIECE,
        T_PIECE,
        S_PIECE,
        Z_PIECE,
        J_PIECE,
        L_PIECE,
        PIECE_COUNT
    };
    
    struct Tetromino {
        TetrominoType type;
        int x, y;
        int rotation;
        bool shape[4][4];
        lv_color_t color;
    };
    
    void createGameScreen();
    void resetGame();
    void spawnTetromino();
    void moveTetromino(int dx, int dy);
    void rotateTetromino();
    void dropTetromino();
    void lockTetromino();
    void checkLines();
    void updateScore();
    void drawBoard();
    void drawTetromino();
    bool isValidPosition(int x, int y, int rotation);
    void initTetrominos();
    void clearNextPieceCanvas();

    static void dropTimerCallback(lv_timer_t* timer);
    
    lv_obj_t* screen_;
    lv_obj_t* boardCanvas_;
    lv_obj_t* scoreLabel_;
    lv_obj_t* nextPieceCanvas_;
    lv_timer_t* dropTimer_;
    
    uint8_t board_[BOARD_HEIGHT][BOARD_WIDTH];
    lv_obj_t* cells_[BOARD_HEIGHT][BOARD_WIDTH];
    
    Tetromino currentPiece_;
    Tetromino nextPiece_;
    Tetromino pieces_[PIECE_COUNT];
    
    int score_;
    int lines_;
    int level_;
    int dropSpeed_;
    bool gameRunning_;
    
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_int_distribution<> pieceDist_;
};

struct RegisterTetris {
    RegisterTetris();
} inline g_registerTetris;