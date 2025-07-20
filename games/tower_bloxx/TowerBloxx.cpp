#include "TowerBloxx.hpp"
#include "GameRegistry.hpp"
#include "lvgl_helper.hpp"
#include "base.h"
#include "blue_centre.h"
#include "red_centre.h"
#include "purple_centre.h"
#include "green_centre.h"
#include <cstdio>
#include <algorithm>
#include "esp_log.h"

static const char *TAG = "TowerBloxx";

RegisterTowerBloxx::RegisterTowerBloxx() {
    GameRegistry::instance().registerGame("Tower Bloxx", [](GameContext& ctx) {
        return std::make_unique<TowerBloxx>();
    });
}

TowerBloxx::TowerBloxx()
   : screen_(nullptr),
     gameContainer_(nullptr),
     scoreLabel_(nullptr),
     towerBase_(nullptr),
     instructionsLabel_(nullptr),
     blocks_(),
     currentBlock_(),
     score_(0),
     towerHeight_(0),
     cameraY_(0),
     firstBlock_(true),
     waitingForPlayer_(false),
     gameRunning_(false),
     blockDropping_(false),
     rd_(),
     gen_(rd_())
{
    ESP_LOGI(TAG, "TowerBloxx constructor called");
    currentBlock_.obj = nullptr;
    currentBlock_.x = 0;
    currentBlock_.y = 0;
    currentBlock_.width = 0;
    currentBlock_.isMoving = false;
    currentBlock_.direction = 1;
}

TowerBloxx::~TowerBloxx() {
    ESP_LOGI(TAG, "TowerBloxx destructor called");
    stop();
}

void TowerBloxx::run() {
    ESP_LOGI(TAG, "TowerBloxx::run() called");
    createGameScreen();
    gameRunning_ = true;      
    resetGame();
    waitingForPlayer_ = true;
    ESP_LOGI(TAG, "TowerBloxx game started, waiting for player input");
}

void TowerBloxx::update() {
    if (!gameRunning_) return;
}

void TowerBloxx::stop() {
    ESP_LOGI(TAG, "TowerBloxx::stop() called");
    
    if (!gameRunning_ && !screen_) {
        ESP_LOGW(TAG, "TowerBloxx already stopped");
        return;
    }
    
    gameRunning_ = false;
    
    ESP_LOGI(TAG, "Stopping all animations");
    lv_anim_del_all();
    
    for (auto& block : blocks_) {
        if (block.obj) {
            lv_obj_del(block.obj);
            block.obj = nullptr;
        }
    }
    blocks_.clear();
    
    if (currentBlock_.obj) {
        lv_obj_del(currentBlock_.obj);
        currentBlock_.obj = nullptr;
    }
    
    if (screen_) {
        ESP_LOGI(TAG, "Deleting screen");
        lv_obj_del(screen_);
        screen_ = nullptr;
        gameContainer_ = nullptr;
        scoreLabel_ = nullptr;
        towerBase_ = nullptr;
        instructionsLabel_ = nullptr;
    }
    
    ESP_LOGI(TAG, "TowerBloxx::stop() completed");
}

void TowerBloxx::handleKey(uint32_t key) {
    ESP_LOGI(TAG, "TowerBloxx::handleKey called with key: %lu", key);
    ESP_LOGI(TAG, "Current state: gameRunning=%d, waitingForPlayer=%d, blockDropping=%d",
           gameRunning_, waitingForPlayer_, blockDropping_);
    ESP_LOGI(TAG, "Current block: obj=%p, isMoving=%d", currentBlock_.obj, currentBlock_.isMoving);
    
    if (!gameRunning_) return;
    
    switch (key) {
        case LV_KEY_ENTER:
        case LV_KEY_DOWN:
            ESP_LOGI(TAG, "Drop key pressed");
            
            if (waitingForPlayer_) {
                ESP_LOGI(TAG, "Creating first block");
                spawnNewBlock();
                waitingForPlayer_ = false;
            } 
            else if (currentBlock_.obj && currentBlock_.isMoving && !blockDropping_) {
                ESP_LOGI(TAG, "Dropping swinging block");
                dropBlock();
            }
            else {
                ESP_LOGI(TAG, "No swinging block to drop or already dropping - ignoring input");
            }
            break;
    }
}

static
void drawPlatformDecor(lv_obj_t* base) {
    for (int i = 0; i < 320; i += 20) {
        lv_obj_t* tile = createCleanObject(base);
        lv_obj_set_size(tile, 18, 18);
        lv_obj_set_pos(tile, i + 1, 60);
        lv_obj_set_style_bg_color(tile, lv_color_make(20, 50, 100), 0);
        lv_obj_set_style_border_width(tile, 1, 0);
        lv_obj_set_style_border_color(tile, lv_color_make(10, 30, 50), 0);
    }

    for (int i = 20; i < 300; i += 80) {
        lv_obj_t* flowerbed = createCleanObject(base);
        lv_obj_set_size(flowerbed, 30, 12);
        lv_obj_set_pos(flowerbed, i, 15);
        lv_obj_set_style_radius(flowerbed, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(flowerbed, lv_color_make(50, 180, 50), 0);
        lv_obj_set_style_border_width(flowerbed, 2, 0);
        lv_obj_set_style_border_color(flowerbed, lv_color_make(30, 100, 30), 0);
    }

    for (int y = 30; y < 60; y += 10) {
        for (int x = (y % 20 == 0 ? 0 : 10); x < 320; x += 20) {
            lv_obj_t* brick = createCleanObject(base);
            lv_obj_set_size(brick, 18, 8);
            lv_obj_set_pos(brick, x, y);
            lv_obj_set_style_bg_color(brick, lv_color_make(40, 70, 150), 0);
            lv_obj_set_style_border_width(brick, 0, 0);
        }
    }
}

void TowerBloxx::createGameScreen() {
    ESP_LOGI(TAG, "TowerBloxx::createGameScreen() called");
    
    screen_ = createCleanObject(nullptr);
    lv_obj_set_style_bg_color(screen_, lv_color_make(235, 206, 135), 0);
    
    gameContainer_ = createCleanObject(screen_);
    lv_obj_set_size(gameContainer_, 320, 480);
    lv_obj_set_pos(gameContainer_, 0, 0);
    lv_obj_set_style_bg_color(gameContainer_, lv_color_make(235, 206, 135), 0);
    lv_obj_set_style_bg_opa(gameContainer_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(gameContainer_, 0, 0);
    lv_obj_set_scrollbar_mode(gameContainer_, LV_SCROLLBAR_MODE_OFF);
    
    scoreLabel_ = lv_label_create(screen_);
    if (scoreLabel_) {
        lv_obj_set_pos(scoreLabel_, 10, 10);
        lv_obj_set_style_text_color(scoreLabel_, lv_color_make(255, 255, 255), 0);
        lv_obj_set_style_text_font(scoreLabel_, &lv_font_montserrat_20, 0);
        lv_obj_set_style_bg_color(scoreLabel_, lv_color_make(0, 0, 0), 0);
        lv_obj_set_style_bg_opa(scoreLabel_, 180, 0);
        lv_obj_set_style_pad_all(scoreLabel_, 5, 0);
        lv_label_set_text(scoreLabel_, "Score: 0\nHeight: 0");
        ESP_LOGI(TAG, "scoreLabel created successfully");
    }
    
    towerBase_ = createCleanObject(gameContainer_);
    lv_obj_set_size(towerBase_, 320, 80);
    lv_obj_set_pos(towerBase_, 0, baseY_);
    lv_obj_set_style_bg_color(towerBase_, lv_color_make(19, 69, 139), 0);
    drawPlatformDecor(towerBase_);

    instructionsLabel_ = lv_label_create(screen_);
    lv_label_set_text(instructionsLabel_, "Press ENTER/DOWN to start!\nESC: Exit");
    lv_obj_align(instructionsLabel_, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(instructionsLabel_, lv_color_make(255, 255, 255), 0);
    lv_obj_set_style_bg_color(instructionsLabel_, lv_color_make(0, 0, 0), 0);
    lv_obj_set_style_bg_opa(instructionsLabel_, 200, 0);
    lv_obj_set_style_pad_all(instructionsLabel_, 10, 0);
    lv_obj_set_style_text_align(instructionsLabel_, LV_TEXT_ALIGN_CENTER, 0);
    
    lv_scr_load(screen_);
    ESP_LOGI(TAG, "Game screen created");
}

void TowerBloxx::resetGame() {
    ESP_LOGI(TAG, "TowerBloxx::resetGame() called");
    
    if (!gameContainer_) {
        ESP_LOGW(TAG, "resetGame called but container null");
        return;
    }
    
    score_ = 0;
    towerHeight_ = 0;
    cameraY_ = 0;
    firstBlock_ = true;
    waitingForPlayer_ = true;
    blockDropping_ = false;

    updateScore();
    
    ESP_LOGI(TAG, "Clearing existing blocks...");
    for (size_t i = 0; i < blocks_.size(); i++) {
        if (blocks_[i].obj) {
            ESP_LOGI(TAG, "Deleting block %zu obj=%p", i, blocks_[i].obj);
            lv_obj_del(blocks_[i].obj);
        }
    }
    blocks_.clear();
    ESP_LOGI(TAG, "All blocks cleared. blocks_.size() = %zu", blocks_.size());
    
    if (currentBlock_.obj) {
        ESP_LOGI(TAG, "Clearing currentBlock_.obj=%p", currentBlock_.obj);
        lv_anim_del(currentBlock_.obj, NULL);
        lv_obj_del(currentBlock_.obj);
        currentBlock_.obj = nullptr;
    }
    
    ESP_LOGI(TAG, "Creating base block...");
    Block base;
    base.x = (lv_disp_get_hor_res(nullptr) - baseBlockWidth_) / 2;
    base.y = baseY_ - blockHeight_;
    base.width    = baseBlockWidth_;
    base.isMoving = false;
    base.direction= 1;

    base.obj = lv_image_create(gameContainer_);
    lv_image_set_src(base.obj, &base_img);
    base.y = baseY_ - lv_obj_get_height(base.obj);
    base.width = lv_obj_get_width(base.obj);

    blocks_.clear();

    ESP_LOGI(TAG, "Base block object created: %p", base.obj);

    lv_obj_set_pos(base.obj, base.x, base.y);

    ESP_LOGI(TAG, "Adding base block to blocks_ vector...");
    blocks_.push_back(base);
    ESP_LOGI(TAG, "Base block added! blocks_.size() = %zu", blocks_.size());
    ESP_LOGI(TAG, "Base block details: obj=%p, pos=(%d,%d), size=%d",
           blocks_[0].obj, blocks_[0].x, blocks_[0].y, blocks_[0].width);
    
    ESP_LOGI(TAG, "Reset game completed successfully");
}

void TowerBloxx::spawnNewBlock() {
    if (!gameRunning_ || !gameContainer_) return;

    if (currentBlock_.obj) {
        lv_anim_del(currentBlock_.obj, NULL);
        lv_obj_del(currentBlock_.obj);
        currentBlock_.obj = nullptr;
    }

    if (firstBlock_ && instructionsLabel_) {
        lv_obj_add_flag(instructionsLabel_, LV_OBJ_FLAG_HIDDEN);
        firstBlock_ = false;
    }

    BlockType type = static_cast<BlockType>(std::uniform_int_distribution<>(0, 3)(gen_));
    const lv_img_dsc_t* img = nullptr;
    switch (type) {
        case BlockType::BlueCentre:   img = &blue_centre_img; break;
        case BlockType::RedCentre:    img = &red_centre_img; break;
        case BlockType::GreenCentre:  img = &green_centre_img; break;
        case BlockType::PurpleCentre: img = &purple_centre_img; break;
    }

    currentBlock_.type = type;

    currentBlock_.obj = lv_image_create(gameContainer_);
    lv_image_set_src(currentBlock_.obj, img);

    currentBlock_.width = lv_obj_get_width(currentBlock_.obj);
    int img_height = lv_obj_get_height(currentBlock_.obj);

    currentBlock_.x = (lv_disp_get_hor_res(nullptr) - currentBlock_.width) / 2;

    const int swingOffset = 60;
    if (!blocks_.empty()) {
        int prevH = lv_obj_get_height(blocks_.back().obj);
        currentBlock_.y = blocks_.back().y - prevH - img_height - swingOffset;
    } else {
        int baseH = lv_obj_get_height(blocks_.front().obj);
        currentBlock_.y = baseY_ - baseH - img_height - swingOffset;
    }

    currentBlock_.isMoving = true;
    currentBlock_.direction = 1;

    lv_obj_set_pos(currentBlock_.obj, currentBlock_.x, currentBlock_.y - cameraY_);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, currentBlock_.obj);
    lv_anim_set_values(&a, 20, 300 - currentBlock_.width - 20);
    lv_anim_set_time(&a, 2000);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)swingAnimationCallback);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_playback_time(&a, 2000);
    lv_anim_set_user_data(&a, this);
    lv_anim_start(&a);

    blockDropping_ = false;
}

void TowerBloxx::dropBlock() {
    if (!currentBlock_.obj || blockDropping_ || !gameRunning_) return;

    blockDropping_ = true;
    lv_anim_del(currentBlock_.obj, nullptr);

    currentBlock_.x = lv_obj_get_x(currentBlock_.obj);
    currentBlock_.isMoving = false;

    int img_height = lv_obj_get_height(currentBlock_.obj);

    int targetY;
    if (!blocks_.empty()) {
        targetY = blocks_.back().y - img_height;
    } else {
        targetY = baseY_ - img_height;
    }

    currentBlock_.y = targetY;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, currentBlock_.obj);
    lv_anim_set_values(&a, lv_obj_get_y(currentBlock_.obj), targetY - cameraY_);
    lv_anim_set_time(&a, 500);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)dropAnimationCallback);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
    lv_anim_set_ready_cb(&a, dropAnimationReadyCallback);
    lv_anim_set_user_data(&a, this);
    lv_anim_start(&a);
}



void TowerBloxx::updateCamera() {
    if (!gameRunning_ || !gameContainer_ || blocks_.empty()) return;

    int worldTopY = blocks_.back().y;

    const int desiredTopOffset = 200;

    if (worldTopY < cameraY_ + desiredTopOffset) {
        cameraY_ = worldTopY - desiredTopOffset;
        ESP_LOGI(TAG, "CameraY updated: %d", cameraY_);

        for (auto& block : blocks_) {
            if (block.obj) {
                lv_obj_del(block.obj);
                block.obj = nullptr;
            }
        }

        for (auto& block : blocks_) {
            block.obj = lv_image_create(gameContainer_);
            switch (block.type) {
                case BlockType::BlueCentre:   lv_image_set_src(block.obj, &blue_centre_img); break;
                case BlockType::RedCentre:    lv_image_set_src(block.obj, &red_centre_img); break;
                case BlockType::GreenCentre:  lv_image_set_src(block.obj, &green_centre_img); break;
                case BlockType::PurpleCentre: lv_image_set_src(block.obj, &purple_centre_img); break;
            }

            lv_obj_set_pos(block.obj, block.x, block.y - cameraY_);
        }

        if (towerBase_) {
            lv_obj_set_y(towerBase_, baseY_ - cameraY_);
        }

        if (currentBlock_.obj && currentBlock_.isMoving) {
            lv_obj_set_y(currentBlock_.obj, currentBlock_.y - cameraY_);
        }

        lv_refr_now(NULL);
    }
}


void TowerBloxx::updateScore() {
    if (!scoreLabel_) {
        ESP_LOGW(TAG, "updateScore called but scoreLabel is null");
        return;
    }
    
    char scoreText[100];
    int result = snprintf(scoreText, sizeof(scoreText), "Score: %d\nHeight: %d", score_, towerHeight_);
    
    if (result < 0 || result >= sizeof(scoreText)) {
        ESP_LOGE(TAG, "Error formatting score text");
        lv_label_set_text(scoreLabel_, "Score: Error");
        return;
    }
    
    lv_label_set_text(scoreLabel_, scoreText);
    ESP_LOGI(TAG, "Score updated: %s", scoreText);
}

void TowerBloxx::swingAnimationCallback(void* obj, int32_t value) {
    lv_obj_set_x(static_cast<lv_obj_t*>(obj), value);
}

void TowerBloxx::dropAnimationCallback(void* obj, int32_t value) {
    lv_obj_set_y(static_cast<lv_obj_t*>(obj), value);
}

void TowerBloxx::dropAnimationReadyCallback(lv_anim_t* a) {
    TowerBloxx* game = static_cast<TowerBloxx*>(a->user_data);
    if (!game || !game->gameRunning_) return;

    game->blockDropping_ = false;

    if (!game->blocks_.empty()) {
        Block& topBlock = game->blocks_.back();

        int blockX = lv_obj_get_x(game->currentBlock_.obj);

        int blockW = lv_obj_get_width(game->currentBlock_.obj);
        int blockH = lv_obj_get_height(game->currentBlock_.obj);

        int topW = lv_obj_get_width(topBlock.obj);

        int leftEdge = std::max(topBlock.x, blockX);
        int rightEdge = std::min(topBlock.x + topW, blockX + blockW);
        int overlap = rightEdge - leftEdge;

        if (overlap > 10) {
            Block newTowerBlock;
            newTowerBlock.type = game->currentBlock_.type;
            newTowerBlock.x = blockX;
            newTowerBlock.y = topBlock.y - blockH;
            newTowerBlock.width = blockW;
            newTowerBlock.isMoving = false;
            newTowerBlock.direction = 1;
            newTowerBlock.obj = game->currentBlock_.obj;
            newTowerBlock.color = game->currentBlock_.color;

            int centerDiff = abs((blockX + blockW / 2) - (topBlock.x + topW / 2));
            game->score_ += std::max(100 - centerDiff, 10);

            game->blocks_.push_back(newTowerBlock);
            game->towerHeight_++;

            game->updateScore();
            game->updateCamera();

            game->currentBlock_ = {};

            lv_timer_t* spawnTimer = lv_timer_create([](lv_timer_t* timer) {
                TowerBloxx* game = static_cast<TowerBloxx*>(lv_timer_get_user_data(timer));
                if (game && game->gameRunning_ && game->gameContainer_) {
                    game->spawnNewBlock();
                }
            }, 600, game);
            lv_timer_set_repeat_count(spawnTimer, 1);
        } else {
            // GAME OVER
            game->gameRunning_ = false;
            if (game->screen_) {
                lv_obj_t* gameOverLabel = lv_label_create(game->screen_);
                lv_label_set_text(gameOverLabel, "GAME OVER!");
                lv_obj_set_style_text_font(gameOverLabel, &lv_font_montserrat_24, 0);
                lv_obj_set_style_text_color(gameOverLabel, lv_color_make(0, 0, 255), 0);
                lv_obj_center(gameOverLabel);

                char finalScoreText[50];
                snprintf(finalScoreText, sizeof(finalScoreText), "Final Score: %d", game->score_);
                lv_obj_t* finalScoreLabel = lv_label_create(game->screen_);
                lv_label_set_text(finalScoreLabel, finalScoreText);
                lv_obj_set_style_text_font(finalScoreLabel, &lv_font_montserrat_20, 0);
                lv_obj_set_style_text_color(finalScoreLabel, lv_color_make(255, 255, 255), 0);
                lv_obj_align(finalScoreLabel, LV_ALIGN_CENTER, 0, 40);
            }
        }
    }
}