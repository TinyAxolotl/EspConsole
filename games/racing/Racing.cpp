#include "Racing.hpp"
#include "GameRegistry.hpp"
#include "core/lv_obj_pos.h"
#include "lvgl_helper.hpp"

#include <cstdio>
#include <algorithm>
#include "esp_log.h"

static const char *TAG = "Racing";


RegisterRacing::RegisterRacing() {
    GameRegistry::instance().registerGame("Racing", [](GameContext& ctx) {
        return std::make_unique<Racing>();
    });
}

Racing::Racing()
  : screen_(nullptr),
    road_(nullptr),
    scoreLabel_(nullptr),
    speedLabel_(nullptr),
    updateTimer_(nullptr),
    score_(0),
    speed_(5),
    lastScore_(0),
    gameRunning_(false),
    lastObstacleY_(-200),
    gen_(rd_()),
    laneDist_(0, 2),
    spawnDist_(0, 100)
{
    player_.obj = nullptr;
    player_.leftWheelsObj = nullptr;
    player_.rightWheelsObj = nullptr;
}

Racing::~Racing() {
   // lv_async_call(&Racing::deferredStop, this);
}

void Racing::run() {
    createGameScreen();
    resetGame();
    gameRunning_ = true;
    
    updateTimer_ = lv_timer_create(gameUpdateTimerCallback, 33, this);
}

void Racing::update() {
    if (!gameRunning_ || !screen_) return;
    
    updateRoad();
    updateObstacles();
    checkCollisions();
    
    if (score_ > lastScore_ && score_ % 200 == 0 && speed_ < 15) {
        speed_ += 2;
        lastScore_ = score_;
    }
}

void Racing::stop() {
    if (stopped_) return;
    stopped_ = true;

    ESP_LOGI(TAG, "Racing::stop() called");
    gameRunning_ = false;

    if (updateTimer_) {
        lv_timer_del(updateTimer_);
        updateTimer_ = nullptr;
    }
    
    cleanupObstacles();
    cleanupPlayer();
    
    for (int lane = 0; lane < laneCount_ - 1; lane++) {
        for (int i = 0; i < 8; i++) {
            if (roadLines_[lane][i] && lv_obj_is_valid(roadLines_[lane][i])) {
                lv_obj_del(roadLines_[lane][i]);
                roadLines_[lane][i] = nullptr;
            }
        }
    }
    
    if (scoreLabel_ && lv_obj_is_valid(scoreLabel_)) {
        lv_obj_del(scoreLabel_);
        scoreLabel_ = nullptr;
    }
    if (speedLabel_ && lv_obj_is_valid(speedLabel_)) {
        lv_obj_del(speedLabel_);
        speedLabel_ = nullptr;
    }
    
    if (road_ && lv_obj_is_valid(road_)) {
        lv_obj_del(road_);
        road_ = nullptr;
    }
    
    if (screen_ && lv_obj_is_valid(screen_)) {
        lv_obj_del(screen_);
        screen_ = nullptr;
    }
    
    ESP_LOGI(TAG, "Racing::stop() completed");
}

void Racing::cleanupPlayer() {
    if (player_.leftWheelsObj && lv_obj_is_valid(player_.leftWheelsObj)) {
        lv_obj_del(player_.leftWheelsObj);
        player_.leftWheelsObj = nullptr;
    }
    if (player_.rightWheelsObj && lv_obj_is_valid(player_.rightWheelsObj)) {
        lv_obj_del(player_.rightWheelsObj);
        player_.rightWheelsObj = nullptr;
    }
    if (player_.obj && lv_obj_is_valid(player_.obj)) {
        lv_obj_del(player_.obj);
        player_.obj = nullptr;
    }
}

void Racing::cleanupObstacles() {
    for (auto& obstacle : obstacles_) {
        cleanupObstacle(obstacle);
    }
    obstacles_.clear();
}

void Racing::cleanupObstacle(Obstacle& obstacle) {
    if (!obstacle.obj) {
        return;
    }

    if (obstacle.obj && lv_obj_is_valid(obstacle.obj)) {
        lv_obj_del(obstacle.obj);
        obstacle.obj = nullptr;
    }

    obstacle.leftWheelsObj = nullptr;
    obstacle.rightWheelsObj = nullptr;
}
void Racing::handleKey(uint32_t key) {
    if (!player_.obj) return;
    
    switch (key) {
        case LV_KEY_LEFT:
            if (gameRunning_) movePlayer(-1);
            break;
            
        case LV_KEY_RIGHT:
            if (gameRunning_) movePlayer(1);
            break;
    }
}

void Racing::createPlayerCar(lv_obj_t* parent) {
    player_.obj = createCleanObject(parent);
    lv_obj_set_size(player_.obj, carWidth_, carHeight_);
    lv_obj_set_style_bg_color(player_.obj, lv_color_make(255, 100, 0), 0);
    lv_obj_set_style_border_width(player_.obj, 0, 0);
    lv_obj_set_style_radius(player_.obj, 0, 0);
    lv_obj_set_style_bg_opa(player_.obj, LV_OPA_TRANSP, 0);
    
    lv_obj_t* centerPart = createCleanObject(player_.obj);
    lv_obj_set_size(centerPart, 30, carHeight_ - 20);
    lv_obj_set_pos(centerPart, 15, 10);
    lv_obj_set_style_bg_color(centerPart, lv_color_make(255, 100, 0), 0);
    lv_obj_set_style_border_width(centerPart, 0, 0);
    lv_obj_set_style_radius(centerPart, 5, 0);
    
    player_.leftWheelsObj = createCleanObject(player_.obj);
    lv_obj_set_size(player_.leftWheelsObj, 15, carHeight_);
    lv_obj_set_pos(player_.leftWheelsObj, 0, 0);
    lv_obj_set_style_bg_color(player_.leftWheelsObj, lv_color_make(150, 50, 0), 0);
    lv_obj_set_style_border_width(player_.leftWheelsObj, 0, 0);
    lv_obj_set_style_radius(player_.leftWheelsObj, 3, 0);
    
    player_.rightWheelsObj = createCleanObject(player_.obj);
    lv_obj_set_size(player_.rightWheelsObj, 15, carHeight_);
    lv_obj_set_pos(player_.rightWheelsObj, 45, 0);
    lv_obj_set_style_bg_color(player_.rightWheelsObj, lv_color_make(150, 50, 0), 0);
    lv_obj_set_style_border_width(player_.rightWheelsObj, 0, 0);
    lv_obj_set_style_radius(player_.rightWheelsObj, 3, 0);
}

void Racing::createObstacleCar(Obstacle& obstacle) {
    if (!road_ || !lv_obj_is_valid(road_)) return;
    
    obstacle.obj = createCleanObject(road_);
    if (!obstacle.obj) return;
    
    lv_obj_set_size(obstacle.obj, obstacleWidth_, obstacleHeight_);
    lv_obj_set_style_bg_opa(obstacle.obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(obstacle.obj, 0, 0);
    
    uint8_t r = std::uniform_int_distribution<>(150, 255)(gen_);
    uint8_t g = std::uniform_int_distribution<>(0, 100)(gen_);
    uint8_t b = std::uniform_int_distribution<>(0, 100)(gen_);
    lv_color_t obstacleColor = lv_color_make(b, g, r);
    
    lv_obj_t* centerPart = createCleanObject(obstacle.obj);
    if (centerPart) {
        lv_obj_set_size(centerPart, 30, obstacleHeight_ - 20);
        lv_obj_set_pos(centerPart, 15, 10);
        lv_obj_set_style_bg_color(centerPart, obstacleColor, 0);
        lv_obj_set_style_border_width(centerPart, 0, 0);
        lv_obj_set_style_radius(centerPart, 5, 0);
    }
    
    obstacle.leftWheelsObj = createCleanObject(obstacle.obj);
    if (obstacle.leftWheelsObj) {
        lv_obj_set_size(obstacle.leftWheelsObj, 15, obstacleHeight_);
        lv_obj_set_pos(obstacle.leftWheelsObj, 0, 0);
        lv_obj_set_style_bg_color(obstacle.leftWheelsObj, lv_color_darken(obstacleColor, 50), 0);
        lv_obj_set_style_border_width(obstacle.leftWheelsObj, 0, 0);
        lv_obj_set_style_radius(obstacle.leftWheelsObj, 3, 0);
    }
    
    obstacle.rightWheelsObj = createCleanObject(obstacle.obj);
    if (obstacle.rightWheelsObj) {
        lv_obj_set_size(obstacle.rightWheelsObj, 15, obstacleHeight_);
        lv_obj_set_pos(obstacle.rightWheelsObj, 45, 0);
        lv_obj_set_style_bg_color(obstacle.rightWheelsObj, lv_color_darken(obstacleColor, 50), 0);
        lv_obj_set_style_border_width(obstacle.rightWheelsObj, 0, 0);
        lv_obj_set_style_radius(obstacle.rightWheelsObj, 3, 0);
    }
}

void Racing::createGameScreen() {
    screen_ = createCleanObject(nullptr);
    lv_obj_set_size(screen_, 320, 480);
    lv_obj_set_style_bg_color(screen_, lv_color_make(50, 50, 50), 0);

    road_ = createCleanObject(screen_);
    lv_obj_set_size(road_, laneWidth_ * laneCount_, 480);
    lv_obj_set_pos(road_, roadStartX_, 0);
    lv_obj_set_style_bg_color(road_, lv_color_make(80, 80, 80), 0);
    
    for (int lane = 0; lane < laneCount_ - 1; lane++) {
        for (int i = 0; i < 8; i++) {
            roadLines_[lane][i] = createCleanObject(road_);
            lv_obj_set_size(roadLines_[lane][i], 4, 40);
            lv_obj_set_pos(roadLines_[lane][i], 
                          laneWidth_ * (lane + 1) - 2, 
                          i * 60);
            lv_obj_set_style_bg_color(roadLines_[lane][i], lv_color_make(255, 255, 255), 0);
            lv_obj_set_style_border_width(roadLines_[lane][i], 0, 0);
            
            roadLineY_[i] = i * 60;
        }
    }
    
    createPlayerCar(road_);
    
    scoreLabel_ = lv_label_create(screen_);
    applyCleanStyle(scoreLabel_);
    lv_obj_set_pos(scoreLabel_, 10, 10);
    lv_obj_set_style_text_color(scoreLabel_, lv_color_make(255, 255, 255), 0);
    lv_obj_set_style_text_font(scoreLabel_, &lv_font_montserrat_20, 0);

    speedLabel_ = lv_label_create(screen_);
    applyCleanStyle(speedLabel_);
    lv_obj_set_pos(speedLabel_, 10, 40);
    lv_obj_set_style_text_color(speedLabel_, lv_color_make(255, 255, 255), 0);
    lv_obj_set_style_text_font(speedLabel_, &lv_font_montserrat_20, 0);
    
    lv_obj_t* instructionsLabel = lv_label_create(screen_);
    applyCleanStyle(instructionsLabel);
    lv_label_set_text(instructionsLabel, "<- -> Move | ESC Exit");
    lv_obj_align(instructionsLabel, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_obj_set_style_text_color(instructionsLabel, lv_color_make(200, 200, 200), 0);
    
    lv_scr_load(screen_);
}

void Racing::resetGame() {
    score_ = 0;
    lastScore_ = 0;
    speed_ = 5;
    lastObstacleY_ = -200;
    
    player_.lane = 1;
    player_.x = laneWidth_ / 2 - carWidth_ / 2 + laneWidth_ * player_.lane;
    player_.y = 360;
    if (player_.obj) {
        lv_obj_set_pos(player_.obj, player_.x, player_.y);
    }
    
    cleanupObstacles();
    
    updateScore();
}

void Racing::updateRoad() {
    if (!road_ || !lv_obj_is_valid(road_)) return;
    
    for (int i = 0; i < 8; i++) {
        roadLineY_[i] += speed_;
        if (roadLineY_[i] > 480) {
            roadLineY_[i] = -40;
        }
        
        for (int lane = 0; lane < laneCount_ - 1; lane++) {
            if (roadLines_[lane][i] && lv_obj_is_valid(roadLines_[lane][i])) {
                lv_obj_set_y(roadLines_[lane][i], roadLineY_[i]);
            }
        }
    }
}

void Racing::updateObstacles() {
    if (!road_ || !lv_obj_is_valid(road_)) return;

    for (auto& obstacle : obstacles_) {
        obstacle.y += speed_;
        if (obstacle.obj && lv_obj_is_valid(obstacle.obj)) {
            lv_obj_set_y(obstacle.obj, obstacle.y);
        }
    }

    std::vector<size_t> toDeleteIndices;
    for (size_t i = 0; i < obstacles_.size(); ++i) {
        if (obstacles_[i].y > 480) {
            toDeleteIndices.push_back(i);
            score_ += 10;
            updateScore();
        }
    }

    for (size_t i : toDeleteIndices) {
        cleanupObstacle(obstacles_[i]);
    }

    for (auto it = obstacles_.rbegin(); it != obstacles_.rend(); ++it) {
        if (it->y > 480) {
            obstacles_.erase(std::next(it).base());
        }
    }

    bool canSpawn = true;

    if (obstacles_.size() >= maxObstaclesOnScreen_) {
        canSpawn = false;
    }

    if (!obstacles_.empty()) {
        int minY = 480;
        for (const auto& obstacle : obstacles_) {
            if (obstacle.y < minY) {
                minY = obstacle.y;
            }
        }

        int minDistance = minObstacleDistance_ - (speed_ * 5);
        if (minDistance < 120) minDistance = 120;

        if (minY < minDistance) {
            canSpawn = false;
        }
    }

    if (canSpawn && spawnDist_(gen_) < 3) {
        spawnObstacle();
    }
}

void Racing::spawnObstacle() {
    if (!road_ || !lv_obj_is_valid(road_)) return;
    
    std::vector<int> occupiedLanes;
    for (const auto& obstacle : obstacles_) {
        if (obstacle.y < 200) {
            occupiedLanes.push_back(obstacle.lane);
        }
    }
    
    if (occupiedLanes.size() >= laneCount_ - 1) {
        return;
    }
    
    int newLane;
    do {
        newLane = laneDist_(gen_);
    } while (std::find(occupiedLanes.begin(), occupiedLanes.end(), newLane) != occupiedLanes.end());
    
    Obstacle obstacle;
    obstacle.lane = newLane;
    obstacle.x = laneWidth_ / 2 - obstacleWidth_ / 2 + laneWidth_ * obstacle.lane;
    obstacle.y = -obstacleHeight_;
    obstacle.obj = nullptr;
    obstacle.leftWheelsObj = nullptr;
    obstacle.rightWheelsObj = nullptr;
    
    createObstacleCar(obstacle);
    
    if (obstacle.obj && lv_obj_is_valid(obstacle.obj)) {
        lv_obj_set_pos(obstacle.obj, obstacle.x, obstacle.y);
        obstacles_.push_back(obstacle);
        lastObstacleY_ = obstacle.y;
    }
}

void Racing::checkCollisions() {
    if (!player_.obj || !lv_obj_is_valid(player_.obj)) return;
    
    for (const auto& obstacle : obstacles_) {
        if (obstacle.obj && lv_obj_is_valid(obstacle.obj) &&
            obstacle.lane == player_.lane &&
            obstacle.y + obstacleHeight_ > player_.y &&
            obstacle.y < player_.y + carHeight_) {
            gameOver();
            return;
        }
    }
}

void Racing::updateScore() {
    if (!scoreLabel_ || !lv_obj_is_valid(scoreLabel_) || 
        !speedLabel_ || !lv_obj_is_valid(speedLabel_)) return;
    
    char scoreText[50];
    snprintf(scoreText, sizeof(scoreText), "Score: %d", score_);
    lv_label_set_text(scoreLabel_, scoreText);
    
    char speedText[50];
    snprintf(speedText, sizeof(speedText), "Speed: %d", speed_);
    lv_label_set_text(speedLabel_, speedText);
}

void Racing::gameOver() {
    gameRunning_ = false;
    
    if (!screen_ || !lv_obj_is_valid(screen_)) return;
    
    lv_obj_t* overlayBg = lv_obj_create(screen_);
    lv_obj_set_size(overlayBg, 320, 480);
    lv_obj_set_pos(overlayBg, 0, 0);
    lv_obj_set_style_bg_color(overlayBg, lv_color_make(0, 0, 0), 0);
    lv_obj_set_style_bg_opa(overlayBg, 220, 0);
    
    lv_obj_t* gameOverLabel = lv_label_create(overlayBg);
    lv_label_set_text(gameOverLabel, "CRASH!");
    lv_obj_set_style_text_font(gameOverLabel, &lv_font_montserrat_26, 0);
    lv_obj_set_style_text_color(gameOverLabel, lv_color_make(0, 0, 255), 0);
    lv_obj_center(gameOverLabel);
    
    char finalScoreText[50];
    snprintf(finalScoreText, sizeof(finalScoreText), "Final Score: %d", score_);
    lv_obj_t* finalScoreLabel = lv_label_create(overlayBg);
    lv_label_set_text(finalScoreLabel, finalScoreText);
    lv_obj_set_style_text_font(finalScoreLabel, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(finalScoreLabel, lv_color_make(255, 255, 255), 0);
    lv_obj_align(finalScoreLabel, LV_ALIGN_CENTER, 0, 40);
    
    lv_obj_t* exitHintLabel = lv_label_create(overlayBg);
    lv_label_set_text(exitHintLabel, "Press ESC to exit");
    lv_obj_set_style_text_font(exitHintLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(exitHintLabel, lv_color_make(200, 200, 200), 0);
    lv_obj_align(exitHintLabel, LV_ALIGN_BOTTOM_MID, 0, -40);
}

void Racing::movePlayer(int direction) {
    if (!player_.obj || !lv_obj_is_valid(player_.obj)) return;
    
    int newLane = player_.lane + direction;
    
    if (newLane >= 0 && newLane < laneCount_) {
        player_.lane = newLane;
        player_.x = laneWidth_ / 2 - carWidth_ / 2 + laneWidth_ * player_.lane;
        lv_obj_set_x(player_.obj, player_.x);
    }
}

void Racing::gameUpdateTimerCallback(lv_timer_t* timer) {
    Racing* game = static_cast<Racing*>(lv_timer_get_user_data(timer));
    if (game && game->gameRunning_) {
        game->update();
    }
}
