#include "ScreenManager.hpp"
#include <cstdio>

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
    
    printf("ScreenManager constructed\n");
}

void ScreenManager::init() {
    if (!initialized_) {
        InputRouter::instance()->setCallback([this](uint32_t key) {
            printf("ScreenManager received key: %lu\n", key);
            handleInput(key);
        });
        
        initialized_ = true;
        printf("ScreenManager initialized\n");
    }
    
    switchToMenu();
}

void ScreenManager::switchToMenu() {
    printf("Switching to Menu\n");
    
    currentGame_.reset();
    
    menuScreen_.show();
    state_ = State::MENU;
}

void ScreenManager::switchToGame(const GameFactory& gameFactory) {
    printf("Switching to Game: %s\n", gameFactory.name.c_str());
    
    currentGame_ = gameFactory.create();
    
    if (currentGame_) {
        currentGame_->run();
        state_ = State::GAME;
    } else {
        printf("Failed to create game\n");
        switchToMenu();
    }
}

void ScreenManager::handleInput(uint32_t key) {
    printf("ScreenManager handling key: %lu, state: %d\n", key, (int)state_);
    
    if (state_ == State::GAME && key == LV_KEY_ESC) {
        switchToMenu();
        return;
    }
    
    if (state_ == State::MENU) {
        printf("Passing key to MenuScreen: %lu\n", key);
        menuScreen_.handleInput(key);
    } else if (state_ == State::GAME && currentGame_) {
        printf("Passing key to Game: %lu\n", key);
        currentGame_->handleKey(key);
    }
}