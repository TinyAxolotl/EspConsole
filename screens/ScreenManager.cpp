#include "ScreenManager.hpp"
#include <cstdio>
#include "esp_log.h"

static const char *TAG = "ScreenManager";

ScreenManager* ScreenManager::instance_ = nullptr;

ScreenManager& ScreenManager::instance() {
    if (!instance_) {
        instance_ = new ScreenManager();
    }
    return *instance_;
}

ScreenManager::ScreenManager() 
{
    menuScreen_.setGameSelectedCallback([this](const GameFactory& gameFactory) {
        switchToGame(gameFactory);
    });
    
    ESP_LOGI(TAG, "ScreenManager constructed");
}

void ScreenManager::init() {
    if (!initialized_) {
        InputRouter::instance()->setCallback([this](uint32_t key) {
            ESP_LOGI(TAG, "ScreenManager received key: %lu", key);
            handleInput(key);
        });
        
        initialized_ = true;
        ESP_LOGI(TAG, "ScreenManager initialized");
    }
    
    switchToMenu();
}

void ScreenManager::switchToMenu() {
    ESP_LOGI(TAG, "Switching to Menu");
    
    if (currentGame_) {
        Game* gameToDelete = currentGame_.release();
        lv_async_call([](void* p) {
            Game* game = static_cast<Game*>(p);
            game->stop();
            delete game;
        }, gameToDelete);
    }
    
    menuScreen_.show();
    state_ = State::MENU;
}

void ScreenManager::switchToGame(const GameFactory& gameFactory) {
    ESP_LOGI(TAG, "Switching to Game: %s", gameFactory.name.c_str());
    
    currentGame_ = gameFactory.create();
    
    if (currentGame_) {
        currentGame_->run();
        state_ = State::GAME;
    } else {
        ESP_LOGE(TAG, "Failed to create game");
        switchToMenu();
    }
}

void ScreenManager::handleInput(uint32_t key) {
    ESP_LOGI(TAG, "ScreenManager handling key: %lu, state: %d", key, (int)state_);

    if (state_ == State::GAME && (key == LV_KEY_ESC || key == LV_KEY_BACKSPACE)) {
        ESP_LOGI(TAG, "Exit from game requested by ScreenManager");
        switchToMenu();
        return;
    }
    
    if (state_ == State::MENU) {
        ESP_LOGI(TAG, "Passing key to MenuScreen: %lu", key);
        menuScreen_.handleInput(key);
    } else if (state_ == State::GAME && currentGame_) {
        ESP_LOGI(TAG, "Passing key to Game: %lu", key);
        currentGame_->handleKey(key);
    }
}