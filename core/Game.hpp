#pragma once
#include <atomic>

class Game {
public:
    virtual ~Game() = default;
    virtual void run() = 0;

    void stop() {
        running = false;
    }

    std::atomic<bool> running{true};
};
