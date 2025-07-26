//#include "assets/background.c"
#include "Arkanoid.hpp"
#include "GameRegistry.hpp"
#include <cstdio>
#include <cmath>
#include "esp_log.h"

static const char *TAG = "Arkanoid";

RegisterArkanoid::RegisterArkanoid() {
    ESP_LOGI(TAG, "Registering Arkanoid game");
    GameRegistry::instance().registerGame("Arkanoid", [](GameContext& ctx) {
        return std::make_unique<Arkanoid>(ctx);
    });
}

Arkanoid::Arkanoid(GameContext& ctx)
    : BaseGame(ctx),
      screen_(nullptr),
      scoreLabel_(nullptr),
      livesLabel_(nullptr),
      score_(0),
      lives_(3),
      level_(1),
      gameRunning_(false),
      ballLaunched_(false),
      enterPressed_(false),
      escPressed_(false),
      gen_(rd_())
{
}

Arkanoid::~Arkanoid() {
    stop();
}

void Arkanoid::onStart() {
    ESP_LOGI(TAG, "Starting Arkanoid game");
    screen_ = screen();
    createGameScreen();
    resetGame();
    gameRunning_ = true;
    
    timers().create(33, gameUpdateTimerCallback, this);
}

void Arkanoid::onUpdate() {
    if (!gameRunning_) return;
    
    uint32_t currentTime = lv_tick_get();
    
    if (leftPressed_ && (currentTime - lastKeyTime_ > keyTimeout_)) {
        leftPressed_ = false;
    }
    if (rightPressed_ && (currentTime - lastKeyTime_ > keyTimeout_)) {
        rightPressed_ = false;
    }
    
    if (leftPressed_) {
        movePaddle(-paddleMoveSpeed_);
    }
    if (rightPressed_) {
        movePaddle(paddleMoveSpeed_);
    }
    
    if (!ballLaunched_) return;
    
    updateBall();
    checkBallCollisions();
    
    bool allDestroyed = true;
    for (const auto& brick : bricks_) {
        if (!brick.destroyed) {
            allDestroyed = false;
            break;
        }
    }
    
    if (allDestroyed) {
        level_++;
        ballLaunched_ = false;
        createLevel();
    }
}

void Arkanoid::stop() {
    if (stopped_) return;
    stopped_ = true;

    ESP_LOGI(TAG, "Arkanoid::stop() called");
    gameRunning_ = false;
    
    
    for (auto& brick : bricks_) {
        if (brick.obj) {
            lv_obj_del(brick.obj);
        }
    }
    bricks_.clear();
    
    screen_ = nullptr;
    BaseGame::stop();
}

void Arkanoid::onInput(uint32_t key) {
    if (!gameRunning_) return;
    
    if (key == LV_KEY_LEFT || key == LV_KEY_RIGHT) {
        lastKeyTime_ = lv_tick_get();
    }
    
    switch (key) {
        case LV_KEY_LEFT:
            leftPressed_ = true;
            rightPressed_ = false;
            break;
            
        case LV_KEY_RIGHT:
            rightPressed_ = true;
            leftPressed_ = false;
            break;
            
        case LV_KEY_ENTER:
        case LV_KEY_UP:
            if (!ballLaunched_ && !enterPressed_) {
                ballLaunched_ = true;
                ball_.vx = ballSpeed_;
                ball_.vy = -ballSpeed_;
                enterPressed_ = true;
                ESP_LOGI(TAG, "Ball launched!");
            }
            break;
            
        case LV_KEY_DOWN:
            enterPressed_ = false;
            break;
            
        case LV_KEY_ESC:
        case LV_KEY_BACKSPACE:
            if (!escPressed_) {
                escPressed_ = true;
                stop();
            }
            break;
            
        default:
            enterPressed_ = false;
            escPressed_ = false;
            break;
    }
}

void Arkanoid::createGameScreen() {
    screen_ = screen();
    lv_obj_set_style_bg_color(screen_, lv_color_make(0, 0, 0), 0);
    
    //LV_IMAGE_DECLARE(background);
    
    //backgroundImg_ = lv_image_create(screen_);
    //lv_image_set_src(backgroundImg_, &background);
    //lv_obj_set_pos(backgroundImg_, 0, 0);
    
    scoreLabel_ = lv_label_create(screen_);
    ui().applyCleanStyle(scoreLabel_);
    lv_obj_set_pos(scoreLabel_, 10, 10);
    lv_obj_set_style_text_color(scoreLabel_, lv_color_make(255, 255, 255), 0);
    lv_obj_set_style_text_font(scoreLabel_, &lv_font_montserrat_20, 0);
    
    livesLabel_ = lv_label_create(screen_);
    ui().applyCleanStyle(livesLabel_);
    lv_obj_set_pos(livesLabel_, 200, 10);
    lv_obj_set_style_text_color(livesLabel_, lv_color_make(255, 255, 255), 0);
    lv_obj_set_style_text_font(livesLabel_, &lv_font_montserrat_20, 0);
    
    lv_obj_t* instructionsLabel = ui().createLabel(screen_, "<- -> Move paddle\n^/ENTER Launch ball", LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_text_color(instructionsLabel, lv_color_make(200, 200, 200), 0);
    lv_obj_set_style_text_align(instructionsLabel, LV_TEXT_ALIGN_CENTER, 0);
    
    paddle_.width = 80;
    paddle_.height = 10;
    paddle_.x = 160 - paddle_.width / 2;
    paddle_.y = 440;
    
    paddle_.obj = lv_obj_create(screen_);
    ui().applyCleanStyle(paddle_.obj);
    lv_obj_set_size(paddle_.obj, paddle_.width, paddle_.height);
    lv_obj_set_pos(paddle_.obj, paddle_.x, paddle_.y);
    lv_obj_set_style_bg_color(paddle_.obj, lv_color_make(255, 255, 255), 0);
    lv_obj_set_style_radius(paddle_.obj, 3, 0);
    
    ball_.size = 8;
    ball_.obj = lv_obj_create(screen_);
    ui().applyCleanStyle(ball_.obj);
    lv_obj_set_size(ball_.obj, ball_.size, ball_.size);
    lv_obj_set_style_bg_color(ball_.obj, lv_color_make(255, 255, 255), 0);
    lv_obj_set_style_radius(ball_.obj, ball_.size / 2, 0);
    
}

void Arkanoid::resetGame() {
    score_ = 0;
    lives_ = 3;
    level_ = 1;
    ballLaunched_ = false;
    enterPressed_ = false;
    escPressed_ = false;
    leftPressed_ = false;
    rightPressed_ = false;
    
    updateScore();
    createLevel();
    
    paddle_.x = 160 - paddle_.width / 2;
    lv_obj_set_x(paddle_.obj, paddle_.x);
    
    ball_.x = paddle_.x + paddle_.width / 2.0f - ball_.size / 2.0f;
    ball_.y = paddle_.y - ball_.size - 2.0f;
    ball_.vx = 0;
    ball_.vy = 0;
    lv_obj_set_pos(ball_.obj, static_cast<int>(ball_.x), static_cast<int>(ball_.y));
}

void Arkanoid::createLevel() {
    for (auto& brick : bricks_) {
        if (brick.obj) {
            lv_obj_del(brick.obj);
        }
    }
    bricks_.clear();
    
    const int rows = (5 + level_ > 8) ? 8 : 5 + level_;
    const int cols = 8;
    const int brickWidth = 35;
    const int brickHeight = 15;
    const int spacing = 2;
    const int startX = (320 - (cols * (brickWidth + spacing))) / 2;
    const int startY = 50;
    
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            Brick brick;
            brick.x = startX + col * (brickWidth + spacing);
            brick.y = startY + row * (brickHeight + spacing);
            brick.width = brickWidth;
            brick.height = brickHeight;
            brick.destroyed = false;
            
            if (row < 2) {
                brick.hits = 3;
                brick.color = lv_color_make(255, 0, 0);
            } else if (row < 4) {
                brick.hits = 2;
                brick.color = lv_color_make(255, 165, 0);
            } else {
                brick.hits = 1;
                brick.color = lv_color_make(0, 255, 0);
            }
            
            brick.obj = lv_obj_create(screen_);
            ui().applyCleanStyle(brick.obj);
            lv_obj_set_size(brick.obj, brick.width, brick.height);
            lv_obj_set_pos(brick.obj, brick.x, brick.y);
            lv_obj_set_style_bg_color(brick.obj, brick.color, 0);
            lv_obj_set_style_border_width(brick.obj, 1, 0);
            lv_obj_set_style_border_color(brick.obj, lv_color_make(128, 128, 128), 0);
            
            bricks_.push_back(brick);
        }
    }
}

void Arkanoid::movePaddle(int dx) {
    paddle_.x += dx;
    
    if (paddle_.x < 0) paddle_.x = 0;
    if (paddle_.x > 320 - paddle_.width) paddle_.x = 320 - paddle_.width;
    
    lv_obj_set_x(paddle_.obj, paddle_.x);
    
    if (!ballLaunched_) {
        ball_.x = paddle_.x + paddle_.width / 2.0f - ball_.size / 2.0f;
        lv_obj_set_x(ball_.obj, static_cast<int>(ball_.x));
    }
}

void Arkanoid::updateBall() {
    ball_.x += ball_.vx;
    ball_.y += ball_.vy;
    
    if (ball_.x <= 0 || ball_.x >= 320 - ball_.size) {
        ball_.vx = -ball_.vx;
        ball_.x = (ball_.x <= 0) ? 0 : 320 - ball_.size;
    }
    
    if (ball_.y <= 40) {
        ball_.vy = -ball_.vy;
        ball_.y = 40;
    }
    
    if (ball_.y > 480) {
        lives_--;
        updateScore();
        
        if (lives_ > 0) {
            ballLaunched_ = false;
            enterPressed_ = false;
            ball_.x = paddle_.x + paddle_.width / 2.0f - ball_.size / 2.0f;
            ball_.y = paddle_.y - ball_.size - 2.0f;
            ball_.vx = 0;
            ball_.vy = 0;
        } else {
            gameOver(false);
        }
    }
    
    lv_obj_set_pos(ball_.obj, static_cast<int>(ball_.x), static_cast<int>(ball_.y));
}

void Arkanoid::checkBallCollisions() {
    checkPaddleCollision(ball_);
    
    for (auto& brick : bricks_) {
        if (!brick.destroyed) {
            checkBrickCollision(ball_, brick);
        }
    }
}

void Arkanoid::checkBrickCollision(Ball& ball, Brick& brick) {
    if (ball.x + ball.size >= brick.x &&
        ball.x <= brick.x + brick.width &&
        ball.y + ball.size >= brick.y &&
        ball.y <= brick.y + brick.height) {
        
        float ballCenterX = ball.x + ball.size / 2.0f;
        float ballCenterY = ball.y + ball.size / 2.0f;
        float brickCenterX = brick.x + brick.width / 2.0f;
        float brickCenterY = brick.y + brick.height / 2.0f;
        
        float dx = ballCenterX - brickCenterX;
        float dy = ballCenterY - brickCenterY;
        
        if (fabs(dx) > fabs(dy)) {
            ball.vx = -ball.vx;
            if (dx > 0) {
                ball.x = brick.x + brick.width;
            } else {
                ball.x = brick.x - ball.size;
            }
        } else {
            ball.vy = -ball.vy;
            if (dy > 0) {
                ball.y = brick.y + brick.height;
            } else {
                ball.y = brick.y - ball.size;
            }
        }
        
        brick.hits--;
        
        if (brick.hits <= 0) {
            brick.destroyed = true;
            lv_obj_del(brick.obj);
            brick.obj = nullptr;
            
            score_ += 10 * level_;
            updateScore();
        } else {
            if (brick.hits == 2) {
                lv_obj_set_style_bg_color(brick.obj, lv_color_make(255, 165, 0), 0);
            } else if (brick.hits == 1) {
                lv_obj_set_style_bg_color(brick.obj, lv_color_make(0, 255, 0), 0);
            }
        }
        
        lv_obj_set_pos(ball.obj, static_cast<int>(ball.x), static_cast<int>(ball.y));
    }
}

void Arkanoid::checkPaddleCollision(Ball& ball) {
    if (ball.x + ball.size >= paddle_.x &&
        ball.x <= paddle_.x + paddle_.width &&
        ball.y + ball.size >= paddle_.y &&
        ball.y <= paddle_.y + paddle_.height) {
        
        ball.vy = -fabs(ball.vy);
        
        float hitPos = (ball.x + ball.size / 2.0f) - paddle_.x;
        float relativePos = hitPos / paddle_.width;
        
        float angle = (relativePos - 0.5f) * 120.0f * 3.14159f / 180.0f;
        float speed = sqrt(ball.vx * ball.vx + ball.vy * ball.vy);
        
        ball.vx = speed * sin(angle);
        ball.vy = -speed * cos(angle);
    }
}

void Arkanoid::updateScore() {
    char scoreText[50];
    snprintf(scoreText, sizeof(scoreText), "Score: %d", score_);
    lv_label_set_text(scoreLabel_, scoreText);
    
    char livesText[50];
    snprintf(livesText, sizeof(livesText), "Lives: %d", lives_);
    lv_label_set_text(livesLabel_, livesText);
}

void Arkanoid::gameOver(bool win) {
    gameRunning_ = false;
    
    lv_obj_t* overlay = lv_obj_create(screen_);
    ui().applyCleanStyle(overlay);
    lv_obj_set_size(overlay, 320, 480);
    lv_obj_set_pos(overlay, 0, 0);
    lv_obj_set_style_bg_color(overlay, lv_color_make(0, 0, 0), 0);
    lv_obj_set_style_bg_opa(overlay, 128, 0);
    
    lv_obj_t* gameOverLabel = lv_label_create(overlay);
    ui().applyCleanStyle(gameOverLabel);
    lv_obj_set_style_text_font(gameOverLabel, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_align(gameOverLabel, LV_TEXT_ALIGN_CENTER, 0);

    if (win) {
        lv_label_set_text(gameOverLabel, "YOU WIN!");
        lv_obj_set_style_text_color(gameOverLabel, lv_color_make(0, 255, 0), 0);
    } else {
        char text[50];
        snprintf(text, sizeof(text), "GAME OVER!\nScore: %d", score_);
        lv_label_set_text(gameOverLabel, text);
        lv_obj_set_style_text_color(gameOverLabel, lv_color_make(255, 0, 0), 0);
    }

    lv_obj_center(gameOverLabel);
    
}

void Arkanoid::gameUpdateTimerCallback(lv_timer_t* timer) {
    Arkanoid* game = static_cast<Arkanoid*>(lv_timer_get_user_data(timer));
    if (game) {
        game->update();
    }
}
