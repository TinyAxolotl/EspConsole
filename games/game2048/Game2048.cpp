#include "Game2048.hpp"
#include "GameRegistry.hpp"
#include "lvgl_helper.hpp"
#include <cstdio>
#include "esp_log.h"

static const char *TAG = "Game2048";
#include <cstring>

Game2048::~Game2048() {
    //stop();
}

Register2048::Register2048() {
    GameRegistry::instance().registerGame("2048", []() {
        return std::make_unique<Game2048>();
    });
}

Game2048::Game2048()
  : score_(0),
    gameRunning_(false),
    won_(false),
    stopped_(false),
    gen_(rd_()),
    posDist_(0, GRID_SIZE - 1),
    valueDist_(1, 10)
{
    std::memset(grid_, 0, sizeof(grid_));
}


void Game2048::run() {
    createGameScreen();
    resetGame();
    gameRunning_ = true;
}

void Game2048::update() {
    if (!gameRunning_) return;
    
    if (!canMove()) {
        gameOver(false);
    }
}


// TODO: Do propper game stopping. Now coredump possibility...
void Game2048::stop() {
    if(stopped_) return;
    stopped_ = true;
    gameRunning_ = false;
    ESP_LOGI(TAG, "2048::stop() called");

    if(screen_ && lv_obj_is_valid(screen_)) {
        lv_obj_clean(screen_);

        lv_obj_del(screen_);
        screen_ = nullptr;
    }

    ESP_LOGI(TAG, "2048::stop() completed");
}

void Game2048::handleKey(uint32_t key) {
    if (!gameRunning_) return;
    
    bool moved = false;
    
    switch (key) {
        case LV_KEY_UP:
            moved = move(0, -1);
            break;
            
        case LV_KEY_DOWN:
            moved = move(0, 1);
            break;
            
        case LV_KEY_LEFT:
            moved = move(-1, 0);
            break;
            
        case LV_KEY_RIGHT:
            moved = move(1, 0);
            break;

    }
    
    if (moved) {
        addRandomTile();
        updateDisplay();
        updateScore();
        
        if (!won_) {
            for (int y = 0; y < GRID_SIZE; y++) {
                for (int x = 0; x < GRID_SIZE; x++) {
                    if (grid_[y][x] == 2048) {
                        won_ = true;
                        gameOver(true);
                        return;
                    }
                }
            }
        }
    }
}

void Game2048::createGameScreen() {
    screen_ = createCleanObject(nullptr);
    lv_obj_set_style_bg_color(screen_, lv_color_make(30, 30, 30), 0);
    
    lv_obj_clear_flag(screen_, LV_OBJ_FLAG_SCROLLABLE);

    scoreLabel_ = lv_label_create(screen_);
    applyCleanStyle(scoreLabel_);
    lv_obj_set_pos(scoreLabel_, 10, 10);
    lv_obj_set_style_text_color(scoreLabel_, lv_color_make(255, 255, 255), 0);
    lv_obj_set_style_text_font(scoreLabel_, &lv_font_montserrat_24, 0);
    
    gameBoard_ = createCleanObject(screen_);
    int boardSize = GRID_SIZE * CELL_SIZE + (GRID_SIZE + 1) * CELL_SPACING;
    lv_obj_set_size(gameBoard_, boardSize, boardSize);
    lv_obj_align(gameBoard_, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_bg_color(gameBoard_, lv_color_make(50, 50, 50), 0);
    lv_obj_set_style_radius(gameBoard_, 8, 0);
    lv_obj_set_style_border_width(gameBoard_, 0, 0);
    
    lv_obj_clear_flag(gameBoard_, LV_OBJ_FLAG_SCROLLABLE);
    
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            tiles_[y][x] = createCleanObject(gameBoard_);
            lv_obj_set_size(tiles_[y][x], CELL_SIZE, CELL_SIZE);
            lv_obj_set_pos(tiles_[y][x], 
                          CELL_SPACING + x * (CELL_SIZE + CELL_SPACING),
                          CELL_SPACING + y * (CELL_SIZE + CELL_SPACING));
            lv_obj_set_style_bg_color(tiles_[y][x], lv_color_make(80, 80, 80), 0);
            lv_obj_set_style_radius(tiles_[y][x], 4, 0);
            lv_obj_set_style_border_width(tiles_[y][x], 0, 0);
            
            tileLabels_[y][x] = lv_label_create(tiles_[y][x]);
            applyCleanStyle(tileLabels_[y][x]);
            lv_label_set_text(tileLabels_[y][x], "");
            lv_obj_center(tileLabels_[y][x]);
            lv_obj_set_style_text_font(tileLabels_[y][x], &lv_font_montserrat_24, 0);
        }
    }
    
    lv_obj_t* instructionsLabel = lv_label_create(screen_);
    applyCleanStyle(instructionsLabel);
    lv_label_set_text(instructionsLabel, "Use arrows to move tiles");
    lv_obj_align(instructionsLabel, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_text_color(instructionsLabel, lv_color_make(200, 200, 200), 0);
    lv_obj_set_style_text_font(instructionsLabel, &lv_font_montserrat_14, 0);
    
    lv_scr_load(screen_);
}

void Game2048::resetGame() {
    score_ = 0;
    won_ = false;
    
    memset(grid_, 0, sizeof(grid_));
    
    addRandomTile();
    addRandomTile();
    
    updateDisplay();
    updateScore();
}

void Game2048::addRandomTile() {

    int emptyCells[GRID_SIZE * GRID_SIZE][2];
    int emptyCount = 0;
    
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            if (grid_[y][x] == 0) {
                emptyCells[emptyCount][0] = x;
                emptyCells[emptyCount][1] = y;
                emptyCount++;
            }
        }
    }
    
    if (emptyCount > 0) {
        int index = std::uniform_int_distribution<>(0, emptyCount - 1)(gen_);
        int x = emptyCells[index][0];
        int y = emptyCells[index][1];
        
        grid_[y][x] = (valueDist_(gen_) > 1) ? 2 : 4;
    }
}

bool Game2048::move(int dx, int dy) {
    bool moved = false;
    int newGrid[GRID_SIZE][GRID_SIZE];
    memcpy(newGrid, grid_, sizeof(grid_));
    
    int xStart = (dx > 0) ? GRID_SIZE - 1 : 0;
    int xEnd = (dx > 0) ? -1 : GRID_SIZE;
    int xStep = (dx > 0) ? -1 : 1;
    
    int yStart = (dy > 0) ? GRID_SIZE - 1 : 0;
    int yEnd = (dy > 0) ? -1 : GRID_SIZE;
    int yStep = (dy > 0) ? -1 : 1;
    
    for (int y = yStart; y != yEnd; y += yStep) {
        for (int x = xStart; x != xEnd; x += xStep) {
            if (newGrid[y][x] == 0) continue;
            
            int newX = x;
            int newY = y;
            
            while (true) {
                int nextX = newX + dx;
                int nextY = newY + dy;
                
                if (nextX < 0 || nextX >= GRID_SIZE || 
                    nextY < 0 || nextY >= GRID_SIZE ||
                    newGrid[nextY][nextX] != 0) {
                    break;
                }
                
                newX = nextX;
                newY = nextY;
            }
            
            int mergeX = newX + dx;
            int mergeY = newY + dy;
            
            if (mergeX >= 0 && mergeX < GRID_SIZE && 
                mergeY >= 0 && mergeY < GRID_SIZE &&
                newGrid[mergeY][mergeX] == newGrid[y][x]) {
                
                newGrid[mergeY][mergeX] *= 2;
                newGrid[y][x] = 0;
                score_ += newGrid[mergeY][mergeX];
                moved = true;
            } else if (newX != x || newY != y) {
                newGrid[newY][newX] = newGrid[y][x];
                newGrid[y][x] = 0;
                moved = true;
            }
        }
    }
    
    memcpy(grid_, newGrid, sizeof(grid_));
    return moved;
}

bool Game2048::canMove() {
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            if (grid_[y][x] == 0) return true;
        }
    }
    
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            int value = grid_[y][x];
            
            if (x < GRID_SIZE - 1 && grid_[y][x + 1] == value) return true;
            if (y < GRID_SIZE - 1 && grid_[y + 1][x] == value) return true;
        }
    }
    
    return false;
}

void Game2048::updateDisplay() {
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            if (grid_[y][x] == 0) {
                lv_label_set_text(tileLabels_[y][x], "");
                lv_obj_set_style_bg_color(tiles_[y][x], lv_color_make(80, 80, 80), 0);
            } else {
                char text[16];
                snprintf(text, sizeof(text), "%d", grid_[y][x]);
                lv_label_set_text(tileLabels_[y][x], text);
                lv_obj_set_style_bg_color(tiles_[y][x], getTileColor(grid_[y][x]), 0);
                
                lv_obj_set_style_text_color(tileLabels_[y][x], lv_color_make(255, 255, 255), 0);
            }
        }
    }
}

void Game2048::updateScore() {
    char scoreText[50];
    snprintf(scoreText, sizeof(scoreText), "Score: %d", score_);
    lv_label_set_text(scoreLabel_, scoreText);
}

void Game2048::gameOver(bool win) {
    gameRunning_ = false;
    
    lv_obj_t* overlayBg = createCleanObject(screen_);
    lv_obj_set_size(overlayBg, 320, 480);
    lv_obj_set_pos(overlayBg, 0, 0);
    lv_obj_set_style_bg_color(overlayBg, lv_color_make(0, 0, 0), 0);
    lv_obj_set_style_bg_opa(overlayBg, 220, 0);
    
    lv_obj_t* gameOverLabel = lv_label_create(overlayBg);
    applyCleanStyle(gameOverLabel);
    if (win) {
        lv_label_set_text(gameOverLabel, "YOU WIN!");
        lv_obj_set_style_text_color(gameOverLabel, lv_color_make(0, 255, 0), 0);
    } else {
        lv_label_set_text(gameOverLabel, "GAME OVER!");
        lv_obj_set_style_text_color(gameOverLabel, lv_color_make(255, 0, 0), 0);
    }
    lv_obj_set_style_text_font(gameOverLabel, &lv_font_montserrat_26, 0);
    lv_obj_center(gameOverLabel);
}

lv_color_t Game2048::getTileColor(int value) {
    switch (value) {
        case 2:    return lv_color_make(255, 0, 0);
        case 4:    return lv_color_make(255, 128, 0);
        case 8:    return lv_color_make(255, 255, 0);
        case 16:   return lv_color_make(128, 255, 0);
        case 32:   return lv_color_make(0, 255, 0);
        case 64:   return lv_color_make(0, 255, 128);
        case 128:  return lv_color_make(0, 255, 255);
        case 256:  return lv_color_make(0, 128, 255);
        case 512:  return lv_color_make(0, 0, 255);
        case 1024: return lv_color_make(128, 0, 255);
        case 2048: return lv_color_make(255, 0, 255);
        default:   return lv_color_make(255, 0, 128);
    }
}