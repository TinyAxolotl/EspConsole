#include "FlappyBird.hpp"
#include "GameRegistry.hpp"
#include "esp_log.h"
#include "lvgl_helper.hpp"
#include "bird.h"
#include <cstdio>
#include <cstring>
#include "esp_heap_caps.h"

RegisterFlappyBird::RegisterFlappyBird() {
    GameRegistry::instance().registerGame("Flappy Bird", [](GameContext& ctx) {
        return std::make_unique<FlappyBird>();
    });
}

FlappyBird::FlappyBird()
  : updateTimer_(nullptr),
    pipes_(),
    birdY_(240),
    birdVelocity_(0),
    score_(0),
    gameRunning_(false),
    gameStarted_(false),
    gen_(rd_()),
    gapDist_(100, 300)
{
}


FlappyBird::~FlappyBird() {
    //stop();
}

static void* dma_image_buffer = nullptr;
static lv_image_dsc_t bird_img_ram;

void FlappyBird::run() {
    createGameScreen();
    resetGame();
    gameRunning_ = true;
    
    updateTimer_ = lv_timer_create(gameUpdateTimerCallback, 16, this);
}

void FlappyBird::update() {
    if (!gameRunning_ || !gameStarted_) return;
    
    updateBird();
    updatePipes();
    checkCollisions();
}

void FlappyBird::stop() {
    gameRunning_ = false;
    
    if (updateTimer_) {
        lv_timer_del(updateTimer_);
        updateTimer_ = nullptr;
    }
    
    for (auto& pipe : pipes_) {
        if (pipe.topObj) lv_obj_del(pipe.topObj);
        if (pipe.bottomObj) lv_obj_del(pipe.bottomObj);
    }
    pipes_.clear();

    if (screen_) {
        lv_obj_del(screen_);
        screen_ = nullptr;
    }

    if (dma_image_buffer) {
        heap_caps_free(dma_image_buffer);
        dma_image_buffer = nullptr;
    }

}

void FlappyBird::handleKey(uint32_t key) {
    if (!gameRunning_) return;
    
    switch (key) {
        case LV_KEY_UP:
        case LV_KEY_ENTER:
            if (!gameStarted_) {
                gameStarted_ = true;
            }
            jump();
            break;
    }
}

void FlappyBird::createGameScreen() {
    screen_ = createCleanObject(nullptr);
    lv_obj_set_style_bg_color(screen_, lv_color_make(0, 102, 255), 0);
    
    groundLine_ = createCleanObject(screen_);
    lv_obj_set_size(groundLine_, 320, 30);
    lv_obj_set_pos(groundLine_, 0, groundY_);
    lv_obj_set_style_bg_color(groundLine_, lv_color_make(19, 69, 139), 0);
    lv_obj_set_style_border_width(groundLine_, 0, 0);


    /* TODO: Asset too big. So it was splitted on half by DMA.
             During investigation was found out that DMA does not work with .rodata,
             But how does green, blue, yellow, etc., asset works - IDK.
             Need to refactor it. Allocate on SPIRAM maybe? */ 
    size_t img_size = bird_img.data_size;
    dma_image_buffer = heap_caps_malloc(img_size, MALLOC_CAP_DMA);
    assert(dma_image_buffer);
    memcpy(dma_image_buffer, bird_img.data, img_size);

    bird_img_ram.header = bird_img.header;
    bird_img_ram.data_size = bird_img.data_size;
    bird_img_ram.data = (const uint8_t*)dma_image_buffer;

    bird_ = lv_image_create(screen_);
    lv_image_set_src(bird_, &bird_img_ram);
    lv_obj_set_pos(bird_, 50, birdY_);

    lv_obj_set_pos(bird_, 50, birdY_);
    
    scoreLabel_ = lv_label_create(screen_);
    applyCleanStyle(scoreLabel_);
    lv_obj_set_pos(scoreLabel_, 10, 10);
    lv_obj_set_style_text_color(scoreLabel_, lv_color_make(0, 0, 0), 0);
    lv_obj_set_style_text_font(scoreLabel_, &lv_font_montserrat_26, 0);
    
    lv_obj_t* instructionsLabel = lv_label_create(screen_);
    applyCleanStyle(instructionsLabel);
    lv_label_set_text(instructionsLabel, "Press UP to flap");
    lv_obj_align(instructionsLabel, LV_ALIGN_CENTER, 0, 100);
    lv_obj_set_style_text_color(instructionsLabel, lv_color_make(0, 0, 0), 0);
    lv_obj_set_style_text_font(instructionsLabel, &lv_font_montserrat_20, 0);
    
    lv_scr_load(screen_);
}

void FlappyBird::resetGame() {
    score_ = 0;
    birdY_ = 240;
    birdVelocity_ = 0;
    gameStarted_ = false;
    
    for (auto& pipe : pipes_) {
        if (pipe.topObj) lv_obj_del(pipe.topObj);
        if (pipe.bottomObj) lv_obj_del(pipe.bottomObj);
    }
    pipes_.clear();
    
    spawnPipe();
    
    lv_obj_set_pos(bird_, 50, birdY_);
    updateScore();
}

void FlappyBird::updateBird() {
    birdVelocity_ += gravity_;
    birdY_ += birdVelocity_;
    
    if (birdY_ < 0) {
        birdY_ = 0;
        birdVelocity_ = 0;
    }
    if (birdY_ > groundY_ - birdSize_) {
        birdY_ = groundY_ - birdSize_;
        gameOver();
    }
    
    lv_obj_set_y(bird_, birdY_);
}

void FlappyBird::updatePipes() {
    for (auto& pipe : pipes_) {
        pipe.x -= pipeSpeed_;
        
        if (pipe.topObj) lv_obj_set_x(pipe.topObj, pipe.x);
        if (pipe.bottomObj) lv_obj_set_x(pipe.bottomObj, pipe.x);

        if (pipe.topObj) lv_obj_invalidate(pipe.topObj);
        if (pipe.bottomObj) lv_obj_invalidate(pipe.bottomObj);

        if (!pipe.passed && pipe.x + pipeWidth_ < 50) {
            pipe.passed = true;
            score_++;
            updateScore();
        }
    }
    
    while (!pipes_.empty() && pipes_.front().x < -pipeWidth_) {
        if (pipes_.front().topObj) lv_obj_del(pipes_.front().topObj);
        if (pipes_.front().bottomObj) lv_obj_del(pipes_.front().bottomObj);
        pipes_.erase(pipes_.begin());
    }
    
    if (pipes_.empty() || pipes_.back().x < 320 - 200) {
        spawnPipe();
    }
}

void FlappyBird::spawnPipe() {
    Pipe pipe;
    pipe.x = 320;
    pipe.gapY = gapDist_(gen_);
    pipe.gapSize = pipeGap_;
    pipe.passed = false;
    
    pipe.topObj = createCleanObject(screen_);
    lv_obj_set_size(pipe.topObj, pipeWidth_, pipe.gapY - pipe.gapSize / 2);
    lv_obj_set_pos(pipe.topObj, pipe.x, 0);
    lv_obj_set_style_bg_color(pipe.topObj, lv_color_make(0, 200, 0), 0);
    lv_obj_set_style_bg_opa(pipe.topObj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(pipe.topObj, 2, 0);
    lv_obj_set_style_border_color(pipe.topObj, lv_color_make(0, 150, 0), 0);

    int bottomY = pipe.gapY + pipe.gapSize / 2;
    int bottomHeight = groundY_ - bottomY;
    pipe.bottomObj = createCleanObject(screen_);
    lv_obj_set_size(pipe.bottomObj, pipeWidth_, bottomHeight);
    lv_obj_set_pos(pipe.bottomObj, pipe.x, bottomY);
    lv_obj_set_style_bg_color(pipe.bottomObj, lv_color_make(0, 200, 0), 0);
    lv_obj_set_style_bg_opa(pipe.bottomObj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(pipe.bottomObj, 2, 0);
    lv_obj_set_style_border_color(pipe.bottomObj, lv_color_make(0, 150, 0), 0);
    
    pipes_.push_back(pipe);
}

void FlappyBird::checkCollisions() {
    int birdX = 50;
    int birdRight = birdX + birdSize_;
    int birdBottom = birdY_ + birdSize_;
    
    for (const auto& pipe : pipes_) {
        if (pipe.x < birdRight && pipe.x + pipeWidth_ > birdX) {
            int gapTop = pipe.gapY - pipe.gapSize / 2;
            int gapBottom = pipe.gapY + pipe.gapSize / 2;
            
            if (birdY_ < gapTop || birdBottom > gapBottom) {
                gameOver();
                return;
            }
        }
    }
}

void FlappyBird::updateScore() {
    char scoreText[20];
    snprintf(scoreText, sizeof(scoreText), "%d", score_);
    lv_label_set_text(scoreLabel_, scoreText);
}

void FlappyBird::gameOver() {
    gameRunning_ = false;
    
    lv_obj_t* gameOverLabel = lv_label_create(screen_);
    applyCleanStyle(gameOverLabel);
    lv_label_set_text(gameOverLabel, "GAME OVER!");
    lv_obj_set_style_text_font(gameOverLabel, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(gameOverLabel, lv_color_make(255, 0, 0), 0);
    lv_obj_center(gameOverLabel);
    
    char finalScoreText[50];
    snprintf(finalScoreText, sizeof(finalScoreText), "Score: %d", score_);
    lv_obj_t* finalScoreLabel = lv_label_create(screen_);
    applyCleanStyle(finalScoreLabel);
    lv_label_set_text(finalScoreLabel, finalScoreText);
    lv_obj_set_style_text_font(finalScoreLabel, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(finalScoreLabel, lv_color_make(255, 0, 0), 0);
    lv_obj_align(finalScoreLabel, LV_ALIGN_CENTER, 0, 40);
}

void FlappyBird::jump() {
    if (gameRunning_) {
        birdVelocity_ = jumpVelocity_;
    }
}

void FlappyBird::gameUpdateTimerCallback(lv_timer_t* timer) {
    FlappyBird* game = static_cast<FlappyBird*>(lv_timer_get_user_data(timer));
    if (game) {
        game->update();
    }
}