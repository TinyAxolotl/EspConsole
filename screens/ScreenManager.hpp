#pragma once

#include "Screen.hpp"
#include "MenuScreen.hpp"
#include "Game.hpp"
#include "InputRouter.hpp"
#include "framework/GameUI.hpp"
#include "framework/TimerManager.hpp"
#include "framework/GameContext.hpp"
#include <memory>

class ScreenManager {
public:
    static ScreenManager& instance();
    
    enum class State {
        MENU,
        GAME
    };
    
    void init();
    void switchToMenu();
    void switchToGame(const GameFactory& gameFactory);
    void handleInput(uint32_t key);

    State state() const { return state_; }
    Game* getCurrentGame() const { return currentGame_.get(); }
private:
    ScreenManager();
    ~ScreenManager() = default;
    
    State state_ = State::MENU;
    
    MenuScreen menuScreen_;
    std::unique_ptr<Game> currentGame_;
    bool initialized_ = false;

    GameUI ui_;
    TimerManager timers_;
    GameContext context_{ui_, timers_, *InputRouter::instance()};
    
    // Singleton
    static ScreenManager* instance_;
};