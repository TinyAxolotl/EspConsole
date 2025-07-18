#include "BaseGame.hpp"

BaseGame::BaseGame(GameContext& ctx)
    : ctx_(ctx) {}

BaseGame::~BaseGame() {
    stop();
}

void BaseGame::run() {
    screen_ = ctx_.ui.createScreen();
    ctx_.input.setCallback([this](uint32_t key) { handleKey(key); });
    updateTimer_ = ctx_.timers.create(16, updateAdapter, this);
    onStart();
    running_ = true;
    screen_.load();
}

void BaseGame::update() {
    if (running_) {
        onUpdate();
    }
}

void BaseGame::stop() {
    running_ = false;
    if (updateTimer_) {
        ctx_.timers.cancel(updateTimer_);
        updateTimer_ = nullptr;
    }
    screen_ = GameUI::Screen();
}

void BaseGame::handleKey(uint32_t key) {
    if (running_) {
        onInput(key);
    }
}

void BaseGame::updateAdapter(lv_timer_t* timer) {
    if (!timer) return;
    BaseGame* self = static_cast<BaseGame*>(timer->user_data);
    if (self) {
        self->update();
    }
}
