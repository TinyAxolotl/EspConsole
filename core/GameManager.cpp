#include "GameManager.hpp"

GameManager::~GameManager() {
    stopCurrentGame();
}

void GameManager::startGame(std::unique_ptr<Game> newGame) {
    std::lock_guard<std::mutex> lock(mutex);
    stopCurrentGame();

    currentGame = std::move(newGame);
    gameThread = std::thread([this](){
        currentGame->run();
    });
}

void GameManager::stopCurrentGame() {
    std::lock_guard<std::mutex> lock(mutex);
    if (currentGame) {
        currentGame->stop();
        if (gameThread.joinable()) {
            gameThread.join();
        }
        currentGame.reset();
    }
}

bool GameManager::isGameRunning() {
    std::lock_guard<std::mutex> lock(mutex);
    return currentGame != nullptr;
}

bool GameManager::currentGameFinished() {
    std::lock_guard<std::mutex> lock(mutex);
    return currentGame == nullptr || !currentGame->running;
}
