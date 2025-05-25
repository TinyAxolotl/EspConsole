#pragma once
#include <cstdint>
#include <string>

class Game {
public:
    virtual ~Game() = default;
    virtual void run() = 0;
    virtual void update() = 0;
    virtual void stop() = 0;
    virtual void handleKey(uint32_t key) = 0;
    virtual std::string name() const = 0;
};
