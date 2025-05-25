#pragma once
#include "Game.hpp"
#include <functional>
#include <vector>
#include <string>
#include <memory>

using CreateGameFn = std::function<std::unique_ptr<Game>()>;

struct GameFactory {
    std::string name;
    CreateGameFn create;
};

// singleton
class GameRegistry {
public:
    static GameRegistry& instance();
    void registerGame(const std::string& name, CreateGameFn fn);
    const std::vector<GameFactory>& available() const;
    void debugPrintGames() const;
private:
    GameRegistry() {
        printf("GameRegistry constructor called\n");
    }
    std::vector<GameFactory> games_;
};