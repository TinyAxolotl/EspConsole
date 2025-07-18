#include "Snake.hpp"
#include "GameRegistry.hpp"
#include "lvgl_helper.hpp"
#include "esp_log.h"
#include <cstdio>
#include <algorithm>
#include <cstring>

RegisterSnake::RegisterSnake() {
    GameRegistry::instance().registerGame("Snake", []() {
        return std::make_unique<Snake>();
    });
}

Snake::Snake()
    : screen_(nullptr),
      scoreLabel_(nullptr),
      updateTimer_(nullptr),
      currentDirection_(DIR_RIGHT),
      nextDirection_(DIR_RIGHT),
      score_(0),
      level_(1),
      foodEaten_(0),
      gameRunning_(false),
      stopped_(false),
      moveSpeed_(INITIAL_SPEED),
      gen_(rd_()),
      xDist_(0, GRID_WIDTH - 1),
      yDist_(0, GRID_HEIGHT - 1)
{
    memset(cells_, 0, sizeof(cells_));
}

Snake::~Snake() {
    stop();
}

void Snake::run() {
    createGameScreen();
    resetGame();
    gameRunning_ = true;
    
    updateTimer_ = lv_timer_create(gameUpdateTimerCallback, moveSpeed_, this);
}

void Snake::update() {
    if (!gameRunning_) return;
    
    currentDirection_ = nextDirection_;
    moveSnake();
    checkCollisions();
    updateDisplay();
}

void Snake::stop() {
    if (stopped_) return;
    stopped_ = true;
    
    gameRunning_ = false;
    
    if (updateTimer_) {
        lv_timer_del(updateTimer_);
        updateTimer_ = nullptr;
    }
    
    if (screen_) {
        lv_obj_del(screen_);
        screen_ = nullptr;
    }
}

void Snake::handleKey(uint32_t key) {
    if (!gameRunning_) return;
    
    switch (key) {
        case LV_KEY_UP:
            if (currentDirection_ != DIR_DOWN) {
                nextDirection_ = DIR_UP;
            }
            break;
            
        case LV_KEY_DOWN:
            if (currentDirection_ != DIR_UP) {
                nextDirection_ = DIR_DOWN;
            }
            break;
            
        case LV_KEY_LEFT:
            if (currentDirection_ != DIR_RIGHT) {
                nextDirection_ = DIR_LEFT;
            }
            break;
            
        case LV_KEY_RIGHT:
            if (currentDirection_ != DIR_LEFT) {
                nextDirection_ = DIR_RIGHT;
            }
            break;
    }
}

void Snake::createGameScreen() {
    ESP_LOGI("MEM", "[Snake Before] Free internal: %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    
    screen_ = createCleanObject(nullptr);
    lv_obj_set_style_bg_color(screen_, lv_color_make(0, 0, 0), 0);
    
    scoreLabel_ = lv_label_create(screen_);
    applyCleanStyle(scoreLabel_);
    lv_obj_set_pos(scoreLabel_, 10, 10);
    lv_obj_set_style_text_color(scoreLabel_, lv_color_make(255, 255, 255), 0);
    lv_obj_set_style_text_font(scoreLabel_, &lv_font_montserrat_20, 0);
    
    lv_obj_t* levelLabel = lv_label_create(screen_);
    applyCleanStyle(levelLabel);
    lv_label_set_text(levelLabel, "Level: 1");
    lv_obj_set_pos(levelLabel, 200, 10);
    lv_obj_set_style_text_color(levelLabel, lv_color_make(255, 255, 255), 0);
    lv_obj_set_style_text_font(levelLabel, &lv_font_montserrat_20, 0);
    
    // Board container
    lv_obj_t* boardContainer = createCleanObject(screen_);
    lv_obj_set_size(boardContainer, GRID_WIDTH * CELL_SIZE + 2, GRID_HEIGHT * CELL_SIZE + 2);
    lv_obj_align(boardContainer, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_bg_color(boardContainer, lv_color_make(32, 32, 32), 0);
    lv_obj_set_style_border_width(boardContainer, 1, 0);
    
    // Create cell template
    lv_obj_t* cellTemplate = createCleanObject(nullptr);
    lv_obj_remove_style_all(cellTemplate);
    lv_obj_set_size(cellTemplate, CELL_SIZE - 1, CELL_SIZE - 1);
    lv_obj_clear_flag(cellTemplate, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(cellTemplate, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_radius(cellTemplate, 2, LV_PART_MAIN);
    lv_obj_set_style_border_width(cellTemplate, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(cellTemplate, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cellTemplate, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cellTemplate, lv_color_make(40, 40, 40), LV_PART_MAIN);
    
    // Create cells
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            cells_[y][x] = lv_obj_create(boardContainer);
            lv_obj_remove_style_all(cells_[y][x]);
            lv_obj_set_size(cells_[y][x], CELL_SIZE - 1, CELL_SIZE - 1);
            lv_obj_set_pos(cells_[y][x], x * CELL_SIZE + 1, y * CELL_SIZE + 1);
            lv_obj_clear_flag(cells_[y][x], LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_clear_flag(cells_[y][x], LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_radius(cells_[y][x], 2, LV_PART_MAIN);
            lv_obj_set_style_border_width(cells_[y][x], 0, LV_PART_MAIN);
            lv_obj_set_style_pad_all(cells_[y][x], 0, LV_PART_MAIN);
            lv_obj_set_style_bg_opa(cells_[y][x], LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_bg_color(cells_[y][x], lv_color_make(40, 40, 40), LV_PART_MAIN);
        }
    }
    
    lv_obj_del(cellTemplate);
    
    lv_obj_t* controlsLabel = lv_label_create(screen_);
    applyCleanStyle(controlsLabel);
    lv_label_set_text(controlsLabel, "<- -> ^ v Move\nESC Exit");
    lv_obj_align(controlsLabel, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_text_color(controlsLabel, lv_color_make(200, 200, 200), 0);
    lv_obj_set_style_text_align(controlsLabel, LV_TEXT_ALIGN_CENTER, 0);
    
    lv_scr_load(screen_);
    
    ESP_LOGI("MEM", "[Snake After] Free internal: %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
}

void Snake::resetGame() {
    score_ = 0;
    level_ = 1;
    foodEaten_ = 0;
    moveSpeed_ = INITIAL_SPEED;
    currentDirection_ = DIR_RIGHT;
    nextDirection_ = DIR_RIGHT;
    
    snake_.clear();
    
    // Initialize snake in the center
    int startX = GRID_WIDTH / 2;
    int startY = GRID_HEIGHT / 2;
    
    for (int i = 0; i < INITIAL_LENGTH; i++) {
        Position pos;
        pos.x = startX - i;
        pos.y = startY;
        snake_.push_back(pos);
    }
    
    spawnFood();
    updateScore();
    updateDisplay();
}

void Snake::moveSnake() {
    if (snake_.empty()) return;
    
    Position newHead = snake_.front();
    
    switch (currentDirection_) {
        case DIR_UP:
            newHead.y--;
            break;
        case DIR_DOWN:
            newHead.y++;
            break;
        case DIR_LEFT:
            newHead.x--;
            break;
        case DIR_RIGHT:
            newHead.x++;
            break;
    }
    
    snake_.push_front(newHead);
    
    if (newHead.x == food_.x && newHead.y == food_.y) {
        score_ += 10;
        foodEaten_++;
        
        if (foodEaten_ % 5 == 0) {
            level_++;
            moveSpeed_ = std::max(50, moveSpeed_ - 50);
            if (updateTimer_) {
                lv_timer_set_period(updateTimer_, moveSpeed_);
            }
        }
        
        updateScore();
        spawnFood();
    } else {
        snake_.pop_back();
    }
}

void Snake::spawnFood() {
    bool validPosition = false;
    
    while (!validPosition) {
        food_.x = xDist_(gen_);
        food_.y = yDist_(gen_);
        
        validPosition = true;
        for (const auto& segment : snake_) {
            if (segment.x == food_.x && segment.y == food_.y) {
                validPosition = false;
                break;
            }
        }
    }
}

void Snake::checkCollisions() {
    if (snake_.empty()) return;
    
    Position head = snake_.front();
    
    if (head.x < 0 || head.x >= GRID_WIDTH || 
        head.y < 0 || head.y >= GRID_HEIGHT) {
        gameOver();
        return;
    }
    
    auto it = snake_.begin();
    ++it;
    for (; it != snake_.end(); ++it) {
        if (it->x == head.x && it->y == head.y) {
            gameOver();
            return;
        }
    }
}

void Snake::updateDisplay() {
    // Clear all cells
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            lv_obj_set_style_bg_color(cells_[y][x], lv_color_make(40, 40, 40), 0);
        }
    }
    
    // Draw snake
    bool isHead = true;
    for (const auto& segment : snake_) {
        if (segment.x >= 0 && segment.x < GRID_WIDTH &&
            segment.y >= 0 && segment.y < GRID_HEIGHT) {
            lv_color_t color = isHead ? lv_color_make(0, 255, 0) : lv_color_make(0, 200, 0);
            lv_obj_set_style_bg_color(cells_[segment.y][segment.x], color, 0);
        }
        isHead = false;
    }
    
    // Draw food
    if (food_.x >= 0 && food_.x < GRID_WIDTH &&
        food_.y >= 0 && food_.y < GRID_HEIGHT) {
        lv_obj_set_style_bg_color(cells_[food_.y][food_.x], lv_color_make(0, 0, 255), 0);
    }
}

void Snake::updateScore() {
    char scoreText[100];
    snprintf(scoreText, sizeof(scoreText), "Score: %d  Level: %d", score_, level_);
    lv_label_set_text(scoreLabel_, scoreText);
}

void Snake::gameOver() {
    gameRunning_ = false;
    
    lv_obj_t* gameOverLabel = lv_label_create(screen_);
    applyCleanStyle(gameOverLabel);
    lv_label_set_text(gameOverLabel, "GAME OVER!");
    lv_obj_set_style_text_font(gameOverLabel, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(gameOverLabel, lv_color_make(0, 0, 255), 0);
    lv_obj_center(gameOverLabel);
    
    char finalScoreText[50];
    snprintf(finalScoreText, sizeof(finalScoreText), "Score: %d", score_);
    lv_obj_t* finalScoreLabel = lv_label_create(screen_);
    applyCleanStyle(finalScoreLabel);
    lv_label_set_text(finalScoreLabel, finalScoreText);
    lv_obj_set_style_text_font(finalScoreLabel, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(finalScoreLabel, lv_color_make(255, 255, 255), 0);
    lv_obj_align(finalScoreLabel, LV_ALIGN_CENTER, 0, 40);
}

void Snake::gameUpdateTimerCallback(lv_timer_t* timer) {
    Snake* game = static_cast<Snake*>(lv_timer_get_user_data(timer));
    if (game && game->gameRunning_) {
        game->update();
    }
}