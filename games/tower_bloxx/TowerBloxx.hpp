#pragma once

#include "Game.hpp"
#include "lvgl.h"
#include <string>
#include <vector>
#include <random>

class TowerBloxx : public Game {
public:
    TowerBloxx();
    ~TowerBloxx() override;

    void run() override;
    void update() override;
    void stop() override;
    void handleKey(uint32_t key) override;
    std::string name() const override { return "Tower Bloxx"; }

private:
    enum class BlockType {
        BlueCentre,
        RedCentre,
        GreenCentre,
        PurpleCentre
    };
    struct Block {
        lv_obj_t* obj;
        int x;
        int y;
        int width;
        bool isMoving;
        int direction;
        lv_color_t color;
        BlockType type;
    };

    void createGameScreen();
    void resetGame();
    void dropBlock();
    void spawnNewBlock();
    void updateCamera();
    void updateScore();
    void updateTowerWobble();

    static void swingAnimationCallback(void* obj, int32_t value);
    static void dropAnimationCallback(void* obj, int32_t value);
    static void dropAnimationReadyCallback(lv_anim_t* a);
    
    lv_obj_t* screen_;
    lv_obj_t* gameContainer_;
    lv_obj_t* scoreLabel_;
    lv_obj_t* towerBase_;
    lv_obj_t* instructionsLabel_;

    lv_anim_t wobbleAnim_;
    int lastWobbleAmplitude_ = 0;

    std::vector<Block> blocks_;
    Block currentBlock_;
    
    int score_;
    int towerHeight_;
    int cameraY_;
    bool firstBlock_;
    bool waitingForPlayer_;
    
    const int blockHeight_ = 20;
    const int baseBlockWidth_ = 60;
    const int baseY_ = 400;
    
    bool gameRunning_;
    bool blockDropping_;
    
    std::random_device rd_;
    std::mt19937 gen_;
};

struct RegisterTowerBloxx {
    RegisterTowerBloxx();
} inline g_registerTowerBloxx;