#include "Tetris.hpp"
#include "GameRegistry.hpp"
#include "esp_log.h"
#include "lvgl_helper.hpp"
#include <cstdio>
#include <cstring>

RegisterTetris::RegisterTetris() {
    GameRegistry::instance().registerGame("Tetris", [](GameContext& ctx) {
        return std::make_unique<Tetris>();
    });
}

Tetris::Tetris()
    : screen_(nullptr),
      boardCanvas_(nullptr),
      scoreLabel_(nullptr),
      nextPieceCanvas_(nullptr),
      dropTimer_(nullptr),
      currentPiece_{},
      nextPiece_{},
      pieces_{},
      score_(0),
      lines_(0),
      level_(1),
      dropSpeed_(1000),
      gameRunning_(false),
      gen_(rd_()),
      pieceDist_(0, PIECE_COUNT - 1)
{
    initTetrominos();
    memset(board_, 0, sizeof(board_));
}


Tetris::~Tetris() {
    stop();
}

void Tetris::initTetrominos() {
    // I-piece (cyan)
    pieces_[I_PIECE].type = I_PIECE;
    pieces_[I_PIECE].color = lv_color_make(255, 255, 0);
    bool i_shape[4][4] = {
        {0,0,0,0},
        {1,1,1,1},
        {0,0,0,0},
        {0,0,0,0}
    };
    memcpy(pieces_[I_PIECE].shape, i_shape, sizeof(i_shape));
    
    // O-piece (yellow)
    pieces_[O_PIECE].type = O_PIECE;
    pieces_[O_PIECE].color = lv_color_make(0, 255, 255);
    bool o_shape[4][4] = {
        {0,0,0,0},
        {0,1,1,0},
        {0,1,1,0},
        {0,0,0,0}
    };
    memcpy(pieces_[O_PIECE].shape, o_shape, sizeof(o_shape));
    
    // T-piece (purple)
    pieces_[T_PIECE].type = T_PIECE;
    pieces_[T_PIECE].color = lv_color_make(128, 0, 128);
    bool t_shape[4][4] = {
        {0,0,0,0},
        {0,1,0,0},
        {1,1,1,0},
        {0,0,0,0}
    };
    memcpy(pieces_[T_PIECE].shape, t_shape, sizeof(t_shape));
    
    // S-piece (green)
    pieces_[S_PIECE].type = S_PIECE;
    pieces_[S_PIECE].color = lv_color_make(0, 255, 0);
    bool s_shape[4][4] = {
        {0,0,0,0},
        {0,1,1,0},
        {1,1,0,0},
        {0,0,0,0}
    };
    memcpy(pieces_[S_PIECE].shape, s_shape, sizeof(s_shape));
    
    // Z-piece (red)
    pieces_[Z_PIECE].type = Z_PIECE;
    pieces_[Z_PIECE].color = lv_color_make(0, 0, 255);
    bool z_shape[4][4] = {
        {0,0,0,0},
        {1,1,0,0},
        {0,1,1,0},
        {0,0,0,0}
    };
    memcpy(pieces_[Z_PIECE].shape, z_shape, sizeof(z_shape));
    
    // J-piece (blue)
    pieces_[J_PIECE].type = J_PIECE;
    pieces_[J_PIECE].color = lv_color_make(255, 0, 0);
    bool j_shape[4][4] = {
        {0,0,0,0},
        {1,0,0,0},
        {1,1,1,0},
        {0,0,0,0}
    };
    memcpy(pieces_[J_PIECE].shape, j_shape, sizeof(j_shape));
    
    // L-piece (orange)
    pieces_[L_PIECE].type = L_PIECE;
    pieces_[L_PIECE].color = lv_color_make(0, 165, 255);
    bool l_shape[4][4] = {
        {0,0,0,0},
        {0,0,1,0},
        {1,1,1,0},
        {0,0,0,0}
    };
    memcpy(pieces_[L_PIECE].shape, l_shape, sizeof(l_shape));
}

void Tetris::run() {
    createGameScreen();
    resetGame();
    gameRunning_ = true;
    spawnTetromino();
    
    dropTimer_ = lv_timer_create(dropTimerCallback, dropSpeed_, this);
}

void Tetris::update() {
    if (!gameRunning_) return;
}

void Tetris::stop() {
    gameRunning_ = false;
    
    if (dropTimer_) {
        lv_timer_del(dropTimer_);
        dropTimer_ = nullptr;
    }
    
    if (screen_) {
        lv_obj_del(screen_);
        screen_ = nullptr;
    }
}

void Tetris::handleKey(uint32_t key) {
    if (!gameRunning_) return;
    
    switch (key) {
        case LV_KEY_LEFT:
            moveTetromino(-1, 0);
            break;
            
        case LV_KEY_RIGHT:
            moveTetromino(1, 0);
            break;
            
        case LV_KEY_DOWN:
            moveTetromino(0, 1);
            break;
            
        case LV_KEY_UP:
            rotateTetromino();
            break;
        case LV_KEY_ENTER:
            dropTetromino();
            break;
            
        case LV_KEY_ESC:
            stop();
            break;
    }
}

lv_obj_t* cloneObject(lv_obj_t* parent, lv_obj_t* templateObj) {
    lv_obj_t* obj = lv_obj_create(parent);

    lv_obj_set_size(obj,
        lv_obj_get_style_width(templateObj, LV_PART_MAIN),
        lv_obj_get_style_height(templateObj, LV_PART_MAIN));

    lv_obj_set_style_bg_color(obj, lv_obj_get_style_bg_color(templateObj, LV_PART_MAIN), LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, lv_obj_get_style_border_width(templateObj, LV_PART_MAIN), LV_PART_MAIN);
    lv_obj_set_style_radius(obj, lv_obj_get_style_radius(templateObj, LV_PART_MAIN), LV_PART_MAIN);
    lv_obj_set_style_pad_all(obj, lv_obj_get_style_pad_top(templateObj, LV_PART_MAIN), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, lv_obj_get_style_bg_opa(templateObj, LV_PART_MAIN), LV_PART_MAIN);

    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);

    return obj;
}

void Tetris::createGameScreen() {
    ESP_LOGI("MEM", "[Before] Free internal: %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

    screen_ = createCleanObject(nullptr);
    lv_obj_set_style_bg_color(screen_, lv_color_make(0, 0, 0), 0);

    lv_obj_t* boardContainer = createCleanObject(screen_);
    lv_obj_set_size(boardContainer, BOARD_WIDTH * CELL_SIZE + 2, BOARD_HEIGHT * CELL_SIZE + 2);
    lv_obj_set_pos(boardContainer, 20, 50);
    lv_obj_set_style_bg_color(boardContainer, lv_color_make(32, 32, 32), 0);
    lv_obj_set_style_border_width(boardContainer, 1, 0);

    lv_obj_t* cellTemplate = createCleanObject(nullptr);
    lv_obj_remove_style_all(cellTemplate);
    lv_obj_set_size(cellTemplate, CELL_SIZE - 1, CELL_SIZE - 1);
    lv_obj_clear_flag(cellTemplate, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(cellTemplate, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_radius(cellTemplate, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_border_width(cellTemplate, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(cellTemplate, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cellTemplate, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cellTemplate, lv_color_make(0, 0, 0), LV_PART_MAIN);

    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            cells_[y][x] = cloneObject(boardContainer, cellTemplate);
            lv_obj_set_parent(cells_[y][x], boardContainer);
            lv_obj_set_pos(cells_[y][x], x * CELL_SIZE + 1, y * CELL_SIZE + 1);
        }
    }

    lv_obj_del(cellTemplate);

    scoreLabel_ = lv_label_create(screen_);
    lv_obj_set_pos(scoreLabel_, 210, 50);
    lv_obj_set_style_text_color(scoreLabel_, lv_color_make(255, 255, 255), 0);
    lv_obj_set_style_text_font(scoreLabel_, &lv_font_montserrat_20, 0);

    lv_obj_t* nextLabel = lv_label_create(screen_);
    lv_label_set_text(nextLabel, "Next:");
    lv_obj_set_pos(nextLabel, 210, 150);
    lv_obj_set_style_text_color(nextLabel, lv_color_make(255, 255, 255), 0);

    nextPieceCanvas_ = createCleanObject(screen_);
    lv_obj_set_size(nextPieceCanvas_, 60, 60);
    lv_obj_set_pos(nextPieceCanvas_, 210, 180);
    lv_obj_set_style_bg_color(nextPieceCanvas_, lv_color_make(32, 32, 32), 0);
    lv_obj_set_style_border_width(nextPieceCanvas_, 0, 0);

    lv_obj_t* controlsLabel = lv_label_create(screen_);
    lv_label_set_text(controlsLabel, "<- -> Move\nv Drop 1 step\nEnter Drop\n^ Rotate");
    lv_obj_set_pos(controlsLabel, 210, 280);
    lv_obj_set_style_text_color(controlsLabel, lv_color_make(200, 200, 200), 0);

    lv_scr_load(screen_);

    ESP_LOGI("MEM", "[After] Free internal: %d", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
}

void Tetris::resetGame() {
    score_ = 0;
    lines_ = 0;
    level_ = 1;
    dropSpeed_ = 1000;
    
    memset(board_, 0, sizeof(board_));
    drawBoard();
    updateScore();
    
    int pieceType = pieceDist_(gen_);
    nextPiece_ = pieces_[pieceType];
}


void Tetris::spawnTetromino() {
    currentPiece_ = nextPiece_;
    currentPiece_.x = BOARD_WIDTH / 2 - 2;
    currentPiece_.y = 0;
    currentPiece_.rotation = 0;

    int pieceType = pieceDist_(gen_);
    nextPiece_ = pieces_[pieceType];

    if (nextPieceCanvas_) {
        lv_obj_clean(nextPieceCanvas_);
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                if (nextPiece_.shape[y][x]) {
                    lv_obj_t* cell = createCleanObject(nextPieceCanvas_);
                    lv_obj_set_size(cell, 12, 12);
                    lv_obj_set_pos(cell, x * 13, y * 13);
                    lv_obj_set_style_bg_color(cell, nextPiece_.color, 0);
                    lv_obj_set_style_border_width(cell, 0, 0);
                }
            }
        }
    }

    if (!isValidPosition(currentPiece_.x, currentPiece_.y, currentPiece_.rotation)) {
        gameRunning_ = false;
        lv_obj_t* gameOverLabel = lv_label_create(screen_);
        lv_label_set_text(gameOverLabel, "GAME OVER!");
        lv_obj_set_style_text_font(gameOverLabel, &lv_font_montserrat_24, 0);
        lv_obj_set_style_text_color(gameOverLabel, lv_color_make(255, 0, 0), 0);
        lv_obj_center(gameOverLabel);
        return;
    }

    drawTetromino();
}

void Tetris::moveTetromino(int dx, int dy) {
    if (isValidPosition(currentPiece_.x + dx, currentPiece_.y + dy, currentPiece_.rotation)) {
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                if (currentPiece_.shape[y][x]) {
                    int boardX = currentPiece_.x + x;
                    int boardY = currentPiece_.y + y;
                    if (boardX >= 0 && boardX < BOARD_WIDTH && boardY >= 0 && boardY < BOARD_HEIGHT) {
                        lv_obj_set_style_bg_color(cells_[boardY][boardX], lv_color_make(0, 0, 0), 0);
                    }
                }
            }
        }
        
        currentPiece_.x += dx;
        currentPiece_.y += dy;
        drawTetromino();
    } else if (dy > 0) {
        lockTetromino();
    }
}

void Tetris::rotateTetromino() {
    if (currentPiece_.type == O_PIECE) return;
    
    int newRotation = (currentPiece_.rotation + 1) % 4;
    
    Tetromino rotated = currentPiece_;
    rotated.rotation = newRotation;
    
    bool newShape[4][4] = {0};
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            newShape[x][3-y] = currentPiece_.shape[y][x];
        }
    }
    memcpy(rotated.shape, newShape, sizeof(newShape));
    
    if (isValidPosition(rotated.x, rotated.y, newRotation)) {
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                if (currentPiece_.shape[y][x]) {
                    int boardX = currentPiece_.x + x;
                    int boardY = currentPiece_.y + y;
                    if (boardX >= 0 && boardX < BOARD_WIDTH && boardY >= 0 && boardY < BOARD_HEIGHT) {
                        lv_obj_set_style_bg_color(cells_[boardY][boardX], lv_color_make(0, 0, 0), 0);
                    }
                }
            }
        }
        
        currentPiece_ = rotated;
        drawTetromino();
    }
}

void Tetris::dropTetromino() {
    while (isValidPosition(currentPiece_.x, currentPiece_.y + 1, currentPiece_.rotation)) {
        moveTetromino(0, 1);
    }
    lockTetromino();
}

void Tetris::lockTetromino() {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (currentPiece_.shape[y][x]) {
                int boardX = currentPiece_.x + x;
                int boardY = currentPiece_.y + y;
                if (boardX >= 0 && boardX < BOARD_WIDTH && boardY >= 0 && boardY < BOARD_HEIGHT) {
                    board_[boardY][boardX] = currentPiece_.type + 1;
                }
            }
        }
    }
    
    checkLines();
    spawnTetromino();
}

void Tetris::checkLines() {
    int linesCleared = 0;
    
    for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
        bool fullLine = true;
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board_[y][x] == 0) {
                fullLine = false;
                break;
            }
        }
        
        if (fullLine) {
            for (int moveY = y; moveY > 0; moveY--) {
                for (int x = 0; x < BOARD_WIDTH; x++) {
                    board_[moveY][x] = board_[moveY - 1][x];
                }
            }
            
            for (int x = 0; x < BOARD_WIDTH; x++) {
                board_[0][x] = 0;
            }
            
            linesCleared++;
            y++;
        }
    }
    
    if (linesCleared > 0) {
        lines_ += linesCleared;
        score_ += linesCleared * 100 * level_;
        
        int newLevel = lines_ / 10 + 1;
        if (newLevel != level_) {
            level_ = newLevel;
            dropSpeed_ = 1000 - (level_ - 1) * 100;
            if (dropSpeed_ < 100) dropSpeed_ = 100;
            
            if (dropTimer_) {
                lv_timer_set_period(dropTimer_, dropSpeed_);
            }
        }
        
        updateScore();
        drawBoard();
    }
}

void Tetris::updateScore() {
    char scoreText[100];
    snprintf(scoreText, sizeof(scoreText), "Score: %d\nLines: %d\nLevel: %d", score_, lines_, level_);
    lv_label_set_text(scoreLabel_, scoreText);
}

void Tetris::drawBoard() {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board_[y][x] > 0) {
                lv_obj_set_style_bg_color(cells_[y][x], pieces_[board_[y][x] - 1].color, 0);
            } else {
                lv_obj_set_style_bg_color(cells_[y][x], lv_color_make(0, 0, 0), 0);
            }
        }
    }
}

void Tetris::drawTetromino() {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (currentPiece_.shape[y][x]) {
                int boardX = currentPiece_.x + x;
                int boardY = currentPiece_.y + y;
                if (boardX >= 0 && boardX < BOARD_WIDTH && boardY >= 0 && boardY < BOARD_HEIGHT) {
                    lv_obj_set_style_bg_color(cells_[boardY][boardX], currentPiece_.color, 0);
                }
            }
        }
    }
}

bool Tetris::isValidPosition(int x, int y, int rotation) {
    for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
            if (currentPiece_.shape[py][px]) {
                int boardX = x + px;
                int boardY = y + py;
                
                if (boardX < 0 || boardX >= BOARD_WIDTH || boardY >= BOARD_HEIGHT) {
                    return false;
                }
                
                if (boardY >= 0 && board_[boardY][boardX] != 0) {
                    return false;
                }
            }
        }
    }
    return true;
}

void Tetris::dropTimerCallback(lv_timer_t* timer) {
    Tetris* game = static_cast<Tetris*>(lv_timer_get_user_data(timer));
    if (game && game->gameRunning_) {
        game->moveTetromino(0, 1);
    }
}