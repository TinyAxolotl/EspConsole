#pragma once

#include "Game.hpp"
#include "lvgl.h"
#include <string>
#include <vector>
#include <random>

class Racing : public Game {
public:
    Racing();
    ~Racing() override;

    void run() override;
    void update() override;
    void stop() override;
    void handleKey(uint32_t key) override;
    std::string name() const override { return "Racing"; }

private:
    struct Car {
        lv_obj_t* obj;
        lv_obj_t* leftWheelsObj;
        lv_obj_t* rightWheelsObj;
        int x;
        int y;
        int lane;
    };
    
    struct Obstacle {
        lv_obj_t* obj;
        lv_obj_t* leftWheelsObj;
        lv_obj_t* rightWheelsObj;
        int x;
        int y;
        int lane;
    };
    
    void createGameScreen();
    void resetGame();
    void updateRoad();
    void updateObstacles();
    void spawnObstacle();
    void checkCollisions();
    void updateScore();
    void gameOver();
    void movePlayer(int direction);
    void createPlayerCar(lv_obj_t* parent);
    void createObstacleCar(Obstacle& obstacle);

    void cleanupPlayer();
    void cleanupObstacles();
    void cleanupObstacle(Obstacle& obstacle);
    
    static void gameUpdateTimerCallback(lv_timer_t* timer);
    
    lv_obj_t* screen_;
    lv_obj_t* road_;
    lv_obj_t* scoreLabel_;
    lv_obj_t* speedLabel_;
    lv_obj_t* roadLines_[3][8];
    lv_timer_t* updateTimer_;
    
    Car player_;
    std::vector<Obstacle> obstacles_;
    
    int score_;
    int speed_;
    int lastScore_;
    int roadLineY_[8];
    bool gameRunning_;
    int lastObstacleY_;
    bool stopped_ = false;

    const int laneWidth_ = 80;
    const int laneCount_ = 3;
    const int carWidth_ = 60;
    const int carHeight_ = 80;
    const int obstacleWidth_ = 60;
    const int obstacleHeight_ = 80;
    const int roadStartX_ = 40;
    const int minObstacleDistance_ = 200;
    const int maxObstaclesOnScreen_ = 3;
    
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_int_distribution<> laneDist_;
    std::uniform_int_distribution<> spawnDist_;
};

struct RegisterRacing {
    RegisterRacing();
} inline g_registerRacing;