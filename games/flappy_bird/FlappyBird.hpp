#pragma once

#include "Game.hpp"
#include "lvgl.h"
#include <string>
#include <vector>
#include <random>

class FlappyBird : public Game {
public:
    FlappyBird();
    ~FlappyBird() override;

    void run() override;
    void update() override;
    void stop() override;
    void handleKey(uint32_t key) override;
    std::string name() const override { return "Flappy Bird"; }

private:
    struct Pipe {
        lv_obj_t* topObj;
        lv_obj_t* bottomObj;
        int x;
        int gapY;
        int gapSize;
        bool passed;
    };
    
    void createGameScreen();
    void resetGame();
    void updateBird();
    void updatePipes();
    void spawnPipe();
    void checkCollisions();
    void updateScore();
    void gameOver();
    void jump();
    
    static void gameUpdateTimerCallback(lv_timer_t* timer);
    
    lv_obj_t* screen_;
    lv_obj_t* bird_;
    lv_obj_t* scoreLabel_;
    lv_obj_t* groundLine_;
    lv_timer_t* updateTimer_;
    
    std::vector<Pipe> pipes_;
    
    float birdY_;
    float birdVelocity_;
    int score_;
    bool gameRunning_;
    bool gameStarted_;
    
    const float gravity_ = 0.5f;
    const float jumpVelocity_ = -8.0f;
    const int birdSize_ = 30;
    const int pipeWidth_ = 50;
    const int pipeGap_ = 200;
    const int pipeSpeed_ = 3;
    const int groundY_ = 450;
    
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_int_distribution<> gapDist_;
};

struct RegisterFlappyBird {
    RegisterFlappyBird();
} inline g_registerFlappyBird;