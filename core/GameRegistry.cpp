#include "GameRegistry.hpp"
#include <cstdio>

GameRegistry& GameRegistry::instance() {
    static GameRegistry inst;
    return inst;
}

void GameRegistry::registerGame(const std::string& name, CreateGameFn fn) {
    printf("GameRegistry::registerGame called for game: %s\n", name.c_str());
    games_.push_back({name, std::move(fn)});
    printf("Games count after registration: %zu\n", games_.size());
}

const std::vector<GameFactory>& GameRegistry::available() const {
    printf("GameRegistry::available() called, games count: %zu\n", games_.size());
    return games_;
}

void GameRegistry::debugPrintGames() const {
    printf("===== REGISTERED GAMES =====\n");
    printf("Number of registered games: %zu\n", games_.size());
    for (size_t i = 0; i < games_.size(); i++) {
        printf("Game %zu: %s\n", i, games_[i].name.c_str());
    }
    printf("============================\n");
}