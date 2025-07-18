#include "app.h" 
#include "InputRouter.hpp"
#include "ScreenManager.hpp"
#include "GameRegistry.hpp"
#include "GamesConnector.hpp"
#include "lvgl.h"
#include <cstdio>
#include "esp_log.h"

static const char *TAG = "MyComponent";

extern "C" void (*handle_input_event)(uint32_t key);

static bool initialized = false;

void process_game_logic(void) {
    if (!initialized) {
        ESP_LOGI(TAG, "Initializing game logic");

        ScreenManager::instance().init();

        initialized = true;
    }

    if (ScreenManager::instance().state() == ScreenManager::State::GAME) {
        Game* currentGame = ScreenManager::instance().getCurrentGame();
        if (currentGame) {
            ESP_LOGI(TAG, "Calling game update()");
            currentGame->update();
        } else {
            ESP_LOGI(TAG, "No active game to update");
        }
    }
}