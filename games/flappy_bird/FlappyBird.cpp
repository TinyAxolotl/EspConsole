#include "FlappyBird.hpp"
#include "GameRegistry.hpp"
#include "esp_log.h"
#include "bird.h"
#include <cstdio>
#include <cstring>
#include "esp_heap_caps.h"

RegisterFlappyBird::RegisterFlappyBird() {
    GameRegistry::instance().registerGame("Flappy Bird", [](GameContext& ctx) {
        return std::make_unique<FlappyBird>(ctx);
    });
}

FlappyBird::FlappyBird(GameContext& ctx)
  : BaseGame(ctx),
    bird_(nullptr),
    scoreLabel_(nullptr),
    groundLine_(nullptr),
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
    stop();
}

static void* dma_image_buffer = nullptr;
static lv_image_dsc_t bird_img_ram;

void FlappyBird::onStart() {
    createGameScreen();
    resetGame();
    gameRunning_ = true;
}

void FlappyBird::onUpdate() {
    if (!gameRunning_ || !gameStarted_) return;

    updateBird();
    updatePipes();
    checkCollisions();
}

void FlappyBird::stop() {
    gameRunning_ = false;
    
    for (auto& pipe : pipes_) {
        if (pipe.topObj) ui().remove(pipe.topObj);
        if (pipe.bottomObj) ui().remove(pipe.bottomObj);
    }
    pipes_.clear();

    if (dma_image_buffer) {
        heap_caps_free(dma_image_buffer);
        dma_image_buffer = nullptr;
    }

    BaseGame::stop();
}

void FlappyBird::onInput(uint32_t key) {
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
    lv_obj_t* scr = screen();
    ui().setBgColor(scr, ui().color(0, 102, 255));

    groundLine_ = ui().createRect(scr, 320, 30, ui().color(19, 69, 139));
    ui().setPos(groundLine_, 0, groundY_);
    ui().setBorder(groundLine_, 0, ui().color(0,0,0));


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

    bird_ = lv_image_create(scr);
    lv_image_set_src(bird_, &bird_img_ram);
    ui().setPos(bird_, 50, birdY_);
    
    scoreLabel_ = ui().createLabel(scr, nullptr);
    ui().setPos(scoreLabel_, 10, 10);
    ui().setTextColor(scoreLabel_, ui().color(0, 0, 0));
    ui().setTextFont(scoreLabel_, &lv_font_montserrat_26);
    
    lv_obj_t* instructionsLabel = ui().createLabel(scr, "Press UP to flap");
    ui().align(instructionsLabel, LV_ALIGN_CENTER, 0, 100);
    ui().setTextColor(instructionsLabel, ui().color(0, 0, 0));
    ui().setTextFont(instructionsLabel, &lv_font_montserrat_20);
}

void FlappyBird::resetGame() {
    score_ = 0;
    birdY_ = 240;
    birdVelocity_ = 0;
    gameStarted_ = false;
    
    for (auto& pipe : pipes_) {
        if (pipe.topObj) ui().remove(pipe.topObj);
        if (pipe.bottomObj) ui().remove(pipe.bottomObj);
    }
    pipes_.clear();
    
    spawnPipe();
    
    ui().setPos(bird_, 50, birdY_);
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
        
        if (pipe.topObj) ui().setX(pipe.topObj, pipe.x);
        if (pipe.bottomObj) ui().setX(pipe.bottomObj, pipe.x);

        if (pipe.topObj) lv_obj_invalidate(pipe.topObj);
        if (pipe.bottomObj) lv_obj_invalidate(pipe.bottomObj);

        if (!pipe.passed && pipe.x + pipeWidth_ < 50) {
            pipe.passed = true;
            score_++;
            updateScore();
        }
    }
    
    while (!pipes_.empty() && pipes_.front().x < -pipeWidth_) {
        if (pipes_.front().topObj) ui().remove(pipes_.front().topObj);
        if (pipes_.front().bottomObj) ui().remove(pipes_.front().bottomObj);
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
    
    lv_obj_t* scr = screen();
    pipe.topObj = ui().createRect(scr, pipeWidth_, pipe.gapY - pipe.gapSize / 2, ui().color(0, 200, 0));
    ui().setPos(pipe.topObj, pipe.x, 0);
    ui().setBgOpacity(pipe.topObj, LV_OPA_COVER);
    ui().setBorder(pipe.topObj, 2, ui().color(0, 150, 0));

    int bottomY = pipe.gapY + pipe.gapSize / 2;
    int bottomHeight = groundY_ - bottomY;
    pipe.bottomObj = ui().createRect(scr, pipeWidth_, bottomHeight, ui().color(0, 200, 0));
    ui().setPos(pipe.bottomObj, pipe.x, bottomY);
    ui().setBgOpacity(pipe.bottomObj, LV_OPA_COVER);
    ui().setBorder(pipe.bottomObj, 2, ui().color(0, 150, 0));
    
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
    ui().setLabelText(scoreLabel_, scoreText);
}

void FlappyBird::gameOver() {
    gameRunning_ = false;

    lv_obj_t* gameOverLabel = ui().createLabel(screen(), nullptr);
    ui().setLabelText(gameOverLabel, "GAME OVER!");
    ui().setTextFont(gameOverLabel, &lv_font_montserrat_24);
    ui().setTextColor(gameOverLabel, ui().color(255, 0, 0));
    ui().center(gameOverLabel);
    
    char finalScoreText[50];
    snprintf(finalScoreText, sizeof(finalScoreText), "Score: %d", score_);
    lv_obj_t* finalScoreLabel = ui().createLabel(screen(), finalScoreText);
    ui().setTextFont(finalScoreLabel, &lv_font_montserrat_20);
    ui().setTextColor(finalScoreLabel, ui().color(255, 0, 0));
    ui().align(finalScoreLabel, LV_ALIGN_CENTER, 0, 40);
}

void FlappyBird::jump() {
    if (gameRunning_) {
        birdVelocity_ = jumpVelocity_;
    }
}
