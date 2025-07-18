#pragma once
#include "Game.hpp"
#include "GameContext.hpp"
#include <string>

class BaseGame : public Game {
public:
    explicit BaseGame(GameContext& ctx);
    ~BaseGame() override;

    void run() override;
    void update() override;
    void stop() override;
    void handleKey(uint32_t key) override;

    virtual std::string name() const override = 0;

protected:
    virtual void onStart() {}
    virtual void onUpdate() {}
    virtual void onInput(uint32_t key) {}

    lv_obj_t* screen() const { return screen_.get(); }
    TimerManager& timers() const { return ctx_.timers; }
    GameUI& ui() const { return ctx_.ui; }
    InputRouter& input() const { return ctx_.input; }

private:
    static void updateAdapter(lv_timer_t* timer);

    GameContext& ctx_;
    GameUI::Screen screen_;
    lv_timer_t* updateTimer_ = nullptr;
    bool running_ = false;
};
