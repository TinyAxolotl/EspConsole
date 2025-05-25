#pragma once

#include "Game.hpp"
#include "lvgl.h"
#include <string>
#include <random>
#include <vector>

class SimpleCatcher : public Game {
public:
    SimpleCatcher();
    ~SimpleCatcher() override;

    void run() override;
    void update() override;
    void stop() override;
    void handleKey(uint32_t key) override;
    std::string name() const override { return "Simple Catcher"; }
    
    void restartGame();
    
    void handleItemCollision(lv_obj_t* itemObj);
    void handleItemMissed();
    
private:
    void createGameScreen();
    void resetGame();
    void spawnItem();
    void checkGameStatus();
    void updateScore();
    
    static void spawnTimerCallback(lv_timer_t* timer);
    static void restartGameTimerCallback(lv_timer_t* timer);
    static void itemAnimationCallback(void* obj, int32_t value);
    static void itemAnimationDeletedCallback(lv_anim_t* a);
    static void playerAnimationCallback(void* obj, int32_t value);
    
    lv_obj_t* screen_ = nullptr;
    lv_obj_t* player_ = nullptr;
    lv_obj_t* scoreLabel_ = nullptr;
    lv_timer_t* spawnTimer_ = nullptr;
    
    bool gameRunning_ = false;
    int playerX_ = 160;
    int playerWidth_ = 80;
    int playerHeight_ = 20;
    int playerSpeed_ = 10;
    int score_ = 0;
    int add_score_lives_ = 0;
    int lives_ = 3;
    unsigned long lastSpawnTime_ = 0;
    unsigned long spawnInterval_ = 1000; // ms
    
    // Falling items
    struct Item {
        lv_obj_t* obj = nullptr;
        int x = 0;
        int y = 0;
        int speed = 0;
    };
    std::vector<Item> items_;
    
    // Random generator
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_int_distribution<> xDist_;
    std::uniform_int_distribution<> speedDist_;
};

// Register the game with the registry
struct RegisterSimpleCatcher {
    RegisterSimpleCatcher();
} inline g_registerSimpleCatcher;