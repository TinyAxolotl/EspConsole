#pragma once

#include "Game.hpp"
#include "lvgl.h"
#include <string>
#include <deque>
#include <random>

class Snake : public Game {
public:
    Snake();
    ~Snake() override;

    void run() override;
    void update() override;
    void stop() override;
    void handleKey(uint32_t key) override;
    std::string name() const override { return "Snake"; }

private:
    static const int GRID_WIDTH = 15;
    static const int GRID_HEIGHT = 15;
    static const int CELL_SIZE = 18;
    static const int INITIAL_LENGTH = 3;
    static const int INITIAL_SPEED = 300;
    
    enum Direction {
        DIR_UP,
        DIR_DOWN,
        DIR_LEFT,
        DIR_RIGHT
    };
    
    struct Position {
        int x;
        int y;
        
        bool operator==(const Position& other) const {
            return x == other.x && y == other.y;
        }
    };
    
    void createGameScreen();
    void resetGame();
    void moveSnake();
    void spawnFood();
    void checkCollisions();
    void updateDisplay();
    void updateScore();
    void gameOver();
    
    static void gameUpdateTimerCallback(lv_timer_t* timer);
    
    lv_obj_t* screen_;
    lv_obj_t* scoreLabel_;
    lv_obj_t* cells_[GRID_HEIGHT][GRID_WIDTH];
    lv_timer_t* updateTimer_;
    
    std::deque<Position> snake_;
    Position food_;
    Direction currentDirection_;
    Direction nextDirection_;
    
    int score_;
    int level_;
    int foodEaten_;
    bool gameRunning_;
    bool stopped_;
    int moveSpeed_;
    
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_int_distribution<> xDist_;
    std::uniform_int_distribution<> yDist_;
};

struct RegisterSnake {
    RegisterSnake();
} inline g_registerSnake;