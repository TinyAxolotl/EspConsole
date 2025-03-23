#include "logic_interface.h"
#include "GameManager.hpp"
#include "main_menu.hpp"

enum class AppState {
    MENU,
    GAME
};

static GameManager gameManager;
static AppState state = AppState::MENU;
static bool menu_initialized = false;

void switch_to_game(std::unique_ptr<Game> game) {
    state = AppState::GAME;
    gameManager.startGame(std::move(game));
}

void switch_to_menu() {
    gameManager.stopCurrentGame();
    state = AppState::MENU;
    menu_initialized = false;
}


void process_game_logic(void) {
    switch (state) {
        case AppState::MENU:
            if (!menu_initialized) {
                initialize_menu(&menu_initialized);
            }
            break;

        case AppState::GAME:
            // Primary game logic (game processing, check game finished\cancelled, save game, etc.)
            if (gameManager.currentGameFinished()) {
                switch_to_menu();
            }
            break;
    }
}