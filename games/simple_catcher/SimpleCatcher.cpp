#include "SimpleCatcher.hpp"
#include "GameRegistry.hpp"
#include <cstdio>
#include <algorithm>

RegisterSimpleCatcher::RegisterSimpleCatcher() {
    printf("Registering Simple Catcher game\n");
    GameRegistry::instance().registerGame("Simple Catcher", []() {
        return std::make_unique<SimpleCatcher>();
    });
}

SimpleCatcher::SimpleCatcher() 
    : gen_(rd_()), 
      xDist_(20, 300),           // Random X position for items
      speedDist_(1, 3)           // Random fall speed (seconds)
{
    // Nothing to do here, screen will be created in run()
}

SimpleCatcher::~SimpleCatcher() {
    stop();
}

void SimpleCatcher::run() {
    printf("Starting Simple Catcher game\n");
    createGameScreen();
    resetGame();
    gameRunning_ = true;
    
    spawnTimer_ = lv_timer_create(spawnTimerCallback, spawnInterval_, this);
}

void SimpleCatcher::update() {
    if (!gameRunning_) return;
    checkGameStatus();
}

void SimpleCatcher::stop() {
    if (!gameRunning_) return;
    
    gameRunning_ = false;
    
    if (spawnTimer_) {
        lv_timer_del(spawnTimer_);
        spawnTimer_ = nullptr;
    }
    
    for (auto& item : items_) {
        if (item.obj) {
            lv_anim_del(item.obj, nullptr);
            lv_obj_del(item.obj);
            item.obj = nullptr;
        }
    }

    items_.clear();
    if (player_) {
        lv_obj_del(player_);
        player_ = nullptr;
    }

    if (scoreLabel_) {
        lv_obj_del(scoreLabel_);
        scoreLabel_ = nullptr;
    }

    if (screen_) {
        lv_obj_del(screen_);
        screen_ = nullptr;
    }
}

void SimpleCatcher::restartGame() {
    if (gameRunning_) {
        if (spawnTimer_) {
            lv_timer_del(spawnTimer_);
            spawnTimer_ = nullptr;
        }
        
        for (auto& item : items_) {
            if (item.obj) {
                lv_anim_del(item.obj, nullptr);
                lv_obj_del(item.obj);
                item.obj = nullptr;
            }
        }
        items_.clear();
    }
    
    resetGame();
    gameRunning_ = true;
    
    spawnTimer_ = lv_timer_create(spawnTimerCallback, spawnInterval_, this);
}

void SimpleCatcher::handleKey(uint32_t key) {
    if (!gameRunning_) return;
    
    switch (key) {
        case LV_KEY_LEFT:
            playerX_ -= playerSpeed_;
            if (playerX_ < 0) playerX_ = 0;
            if (player_) {
                lv_obj_set_x(player_, playerX_);
            }
            break;
            
        case LV_KEY_RIGHT:
            playerX_ += playerSpeed_;
            if (playerX_ > 320 - playerWidth_) playerX_ = 320 - playerWidth_;
            if (player_) {
                lv_obj_set_x(player_, playerX_);
            }
            break;
            
        case LV_KEY_ESC:
            SimpleCatcher::stop();
            
            break;
    }
}

void SimpleCatcher::createGameScreen() {
    // Create game screen
    screen_ = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen_, lv_color_make(0, 0, 0), 0);
    
    // Create score label
    scoreLabel_ = lv_label_create(screen_);
    lv_obj_set_pos(scoreLabel_, 10, 10);
    lv_obj_set_style_text_color(scoreLabel_, lv_color_make(255, 255, 255), 0);
    
    // Create player paddle
    player_ = lv_obj_create(screen_);
    lv_obj_set_size(player_, playerWidth_, playerHeight_);
    lv_obj_set_pos(player_, playerX_, 440);  // Bottom of screen
    lv_obj_set_style_bg_color(player_, lv_color_make(0, 0, 255), 0);
    
    // Load the screen
    lv_scr_load(screen_);
}

void SimpleCatcher::resetGame() {
    score_ = 0;
    lives_ = 3;
    updateScore();
    
    // Reset player position
    playerX_ = 160 - playerWidth_ / 2;
    if (player_) {
        lv_obj_set_pos(player_, playerX_, 440);
    }
    
    lastSpawnTime_ = lv_tick_get();
}

void SimpleCatcher::spawnItem() {
    if (!gameRunning_) return;
    
    // Create a new falling item
    Item newItem;
    newItem.x = xDist_(gen_);
    newItem.y = 0;
    newItem.speed = speedDist_(gen_);
    
    // Create the visual object
    newItem.obj = lv_obj_create(screen_);
    lv_obj_set_size(newItem.obj, 20, 20);
    lv_obj_set_pos(newItem.obj, newItem.x, 0);
    
    lv_obj_set_user_data(newItem.obj, this);
    
    // Randomize color
    uint8_t r = std::uniform_int_distribution<>(50, 255)(gen_);
    uint8_t g = std::uniform_int_distribution<>(50, 255)(gen_);
    uint8_t b = std::uniform_int_distribution<>(50, 255)(gen_);
    lv_obj_set_style_bg_color(newItem.obj, lv_color_make(r, g, b), 0);
    
    // Create animation for falling
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, newItem.obj);
    lv_anim_set_values(&a, 0, 500);
    lv_anim_set_time(&a, newItem.speed * 1000);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)itemAnimationCallback);
    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_set_ready_cb(&a, itemAnimationDeletedCallback);
    
    lv_anim_set_user_data(&a, this);
    
    lv_anim_start(&a);
    
    items_.push_back(newItem);
    
    items_.erase(
        std::remove_if(items_.begin(), items_.end(), 
            [](const Item& item) { return item.obj == nullptr; }),
        items_.end()
    );
}

void SimpleCatcher::checkGameStatus() {
    if (lives_ <= 0 && gameRunning_) {
        gameRunning_ = false;
        
        // Simple game over message
        lv_obj_t* gameOverLabel = lv_label_create(screen_);
        lv_label_set_text(gameOverLabel, "GAME OVER!");
        lv_obj_set_style_text_font(gameOverLabel, &lv_font_montserrat_24, 0);
        lv_obj_set_style_text_color(gameOverLabel, lv_color_make(255, 0, 0), 0);
        lv_obj_center(gameOverLabel);

        if (spawnTimer_) {
            lv_timer_del(spawnTimer_);
            spawnTimer_ = nullptr;
        }

        lv_timer_t* restartTimer = lv_timer_create(restartGameTimerCallback, 3000, this);
        lv_timer_set_repeat_count(restartTimer, 1);
    }
}

void SimpleCatcher::updateScore() {
    if (((score_ - add_score_lives_) % 100) > 1) {
        lives_++;
        add_score_lives_ = score_;
    }

    if (scoreLabel_) {
        char scoreText[50];
        snprintf(scoreText, sizeof(scoreText), "Score: %d   Lives: %d", score_, lives_);
        lv_label_set_text(scoreLabel_, scoreText);
    }
}

void SimpleCatcher::handleItemCollision(lv_obj_t* itemObj) {
    if (!gameRunning_) return;
    
    auto it = std::find_if(items_.begin(), items_.end(), 
                          [itemObj](const Item& item) { return item.obj == itemObj; });
    
    if (it != items_.end()) {
        lv_anim_del(itemObj, nullptr);
        
        lv_obj_del(itemObj);
        it->obj = nullptr;
        
        score_ += 10;
        updateScore();
    }
}

void SimpleCatcher::handleItemMissed() {
    if (!gameRunning_) return;

    lives_--;
    updateScore();
}

void SimpleCatcher::spawnTimerCallback(lv_timer_t* timer) {
    SimpleCatcher* game = static_cast<SimpleCatcher*>(lv_timer_get_user_data(timer));
    if (game && game->gameRunning_) {
        game->spawnItem();
    }
}

void SimpleCatcher::restartGameTimerCallback(lv_timer_t* timer) {
    SimpleCatcher* game = static_cast<SimpleCatcher*>(lv_timer_get_user_data(timer));
    if (game) {
        game->restartGame();
    }
}

void SimpleCatcher::itemAnimationCallback(void* obj, int32_t value) {
    lv_obj_set_y(static_cast<lv_obj_t*>(obj), value);

    lv_anim_t* a = nullptr;

    lv_obj_t* item = static_cast<lv_obj_t*>(obj);
    void* userData = lv_obj_get_user_data(item);
    SimpleCatcher* game = static_cast<SimpleCatcher*>(userData);

    if (game && game->player_ && game->gameRunning_) {
        lv_obj_t* player = game->player_;
        lv_obj_t* item = static_cast<lv_obj_t*>(obj);

        int playerX = lv_obj_get_x(player);
        int playerY = lv_obj_get_y(player);
        int playerWidth = lv_obj_get_width(player);
        int playerHeight = lv_obj_get_height(player);

        int itemX = lv_obj_get_x(item);
        int itemY = value;
        int itemWidth = lv_obj_get_width(item);
        int itemHeight = lv_obj_get_height(item);
    
        bool collision = 
            itemY + itemHeight >= playerY && 
            itemY <= playerY + playerHeight &&
            itemX + itemWidth >= playerX && 
            itemX <= playerX + playerWidth;

        if (collision) {
            game->handleItemCollision(item);
        }
    }
}

void SimpleCatcher::itemAnimationDeletedCallback(lv_anim_t* a) {
    SimpleCatcher* game = static_cast<SimpleCatcher*>(a->user_data);
    
    if (game && game->gameRunning_) {
        for (auto& item : game->items_) {
            if (item.obj) {
                int itemY = lv_obj_get_y(item.obj);
                
                if (itemY >= 480) {
                    lv_obj_del(item.obj);
                    item.obj = nullptr;
                    
                    game->handleItemMissed();
                    break;
                }
            }
        }
    }
}

void SimpleCatcher::playerAnimationCallback(void* obj, int32_t value) {
    lv_obj_set_x(static_cast<lv_obj_t*>(obj), value);
}