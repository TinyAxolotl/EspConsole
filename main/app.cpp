#include "app.h" 
#include "InputRouter.hpp"
#include "ScreenManager.hpp"
#include "GameRegistry.hpp"
#include "GamesConnector.hpp"
#include "lvgl.h"
#include <cstdio>

extern "C" void (*handle_input_event)(uint32_t key);

static bool initialized = false;

void process_game_logic(void) {
    if (!initialized) {
        printf("Initializing game logic\n");

        ScreenManager::instance().init();

        initialized = true;
    }

    if (ScreenManager::instance().state() == ScreenManager::State::GAME) {
        Game* currentGame = ScreenManager::instance().getCurrentGame();
        if (currentGame) {
            printf("Calling game update()\n");
            currentGame->update();
        } else {
            printf("No active game to update\n");
        }
    }
}