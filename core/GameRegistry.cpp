#include "GameRegistry.hpp"
#include <cstdio>
#include "esp_log.h"

static const char *TAG = "GameRegistry";

GameRegistry& GameRegistry::instance() {
    static GameRegistry inst;
    return inst;
}

void GameRegistry::registerGame(const std::string& name, CreateGameFn fn) {
    ESP_LOGI(TAG, "GameRegistry::registerGame called for game: %s", name.c_str());
    games_.push_back({name, std::move(fn)});
    ESP_LOGI(TAG, "Games count after registration: %zu", games_.size());
}

const std::vector<GameFactory>& GameRegistry::available() const {
    ESP_LOGI(TAG, "GameRegistry::available() called, games count: %zu", games_.size());
    return games_;
}

void GameRegistry::debugPrintGames() const {
    ESP_LOGI(TAG, "===== REGISTERED GAMES =====");
    ESP_LOGI(TAG, "Number of registered games: %zu", games_.size());
    for (size_t i = 0; i < games_.size(); i++) {
        ESP_LOGI(TAG, "Game %zu: %s", i, games_[i].name.c_str());
    }
    ESP_LOGI(TAG, "============================");
}