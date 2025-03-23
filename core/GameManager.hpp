#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include "Game.hpp"

class GameManager {
public:
    GameManager() = default;
    ~GameManager();

    void startGame(std::unique_ptr<Game> newGame);
    void stopCurrentGame();
    bool isGameRunning();
    bool currentGameFinished();

private:
    std::unique_ptr<Game> currentGame = nullptr;
    std::thread gameThread;
    std::mutex mutex;
};
