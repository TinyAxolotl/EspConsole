#pragma once

#include "BaseGame.hpp"
#include "lvgl.h"
#include <string>
#include <vector>
#include <random>

class Arkanoid : public BaseGame {
public:
    explicit Arkanoid(GameContext& ctx);
    ~Arkanoid() override;

    void onStart() override;
    void onUpdate() override;
    void onInput(uint32_t key) override;
    void stop() override;
    std::string name() const override { return "Arkanoid"; }

private:
    struct Brick {
        lv_obj_t* obj;
        //lv_obj_t* img;
        int x, y;
        int width, height;
        int hits;
        bool destroyed;
        lv_color_t color;
    };
    
    struct Ball {
        lv_obj_t* obj;
        //lv_obj_t* img;
        float x, y;
        float vx, vy;
        int size;
    };
    
    struct Paddle {
        lv_obj_t* obj;
        //lv_obj_t* img;
        int x, y;
        int width, height;
    };
    
    void createGameScreen();
    void resetGame();
    void createLevel();
    void movePaddle(int dx);
    void updateBall();
    void checkBallCollisions();
    void checkBrickCollision(Ball& ball, Brick& brick);
    void checkPaddleCollision(Ball& ball);
    void updateScore();
    void gameOver(bool win);
    
    static void gameUpdateTimerCallback(lv_timer_t* timer);
    
    lv_obj_t* screen_;
    lv_obj_t* scoreLabel_;
    lv_obj_t* livesLabel_;
    
    Paddle paddle_;
    Ball ball_;
    std::vector<Brick> bricks_;
    
    int score_;
    int lives_;
    int level_;
    bool gameRunning_;
    bool ballLaunched_;
    
    bool stopped_ = false;
    bool enterPressed_ = false;
    bool escPressed_ = false;
    
    bool leftPressed_ = false;
    bool rightPressed_ = false;
    uint32_t lastKeyTime_ = 0;
    const uint32_t keyTimeout_ = 500;
    
    const int paddleSpeed_ = 15;
    const float ballSpeed_ = 3.0f;
    const float paddleMoveSpeed_ = 5.0f;
    
    std::random_device rd_;
    std::mt19937 gen_;

    lv_obj_t* backgroundImg_;
};

struct RegisterArkanoid {
    RegisterArkanoid();
} inline g_registerArkanoid;