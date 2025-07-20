#include "Minesweeper.hpp"
#include "GameRegistry.hpp"
#include "lvgl_helper.hpp"
#include <cstdio>
#include <algorithm>
#include <queue>
#include "lvgl/src/misc/lv_timer.h"

RegisterMinesweeper::RegisterMinesweeper() {
    GameRegistry::instance().registerGame("Minesweeper", [](GameContext& ctx) {
        return std::make_unique<Minesweeper>();
    });
}

Minesweeper::Minesweeper()
    : cursorX_(0),
      cursorY_(0),
      flagCount_(0),
      revealedCount_(0),
      gameRunning_(false),
      firstClick_(true),
      rd_(),
      gen_(rd_())
{
}


Minesweeper::~Minesweeper() {
    //stop();
}

void Minesweeper::run() {
    createGameScreen();
    resetGame();
    gameRunning_ = true;
}

void Minesweeper::update() {
    if (!gameRunning_) return;
}

void Minesweeper::stop() {
    if (stopped_ == true) return;
    stopped_ = true;
    gameRunning_ = false;
    
    if (screen_) {
        lv_obj_del(screen_);
        screen_ = nullptr;
    }
}

void Minesweeper::processRevealStep() {
    int steps = 0;
    const int MAX_STEPS = 10;

    while (!revealQueue_.empty() && steps < MAX_STEPS) {
        auto [x, y] = revealQueue_.front();
        revealQueue_.pop();
        steps++;

        if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) continue;

        Cell& cell = grid_[y][x];

        if (cell.state == REVEALED) continue;

        if (cell.state == FLAGGED) {
            cell.state = HIDDEN;
            totalFlags_--;
            lv_label_set_text(cell.label, "");
            lv_obj_set_style_bg_color(cell.obj, lv_color_make(180, 180, 180), 0);
            updateDisplay();
        }

        if (cell.state != HIDDEN) continue;

        cell.state = REVEALED;
        revealedCount_++;

        if (cell.hasMine) {
            lv_label_set_text(cell.label, "*");
            lv_obj_set_style_bg_color(cell.obj, lv_color_make(255, 0, 0), 0);
            lv_obj_set_style_text_color(cell.label, lv_color_make(255, 255, 255), 0);
            revealAllMines();
            gameOver(false);
            lv_timer_del(revealTimer_);
            revealTimer_ = nullptr;
            return;
        }

        lv_obj_set_style_bg_color(cell.obj, lv_color_make(140, 140, 140), 0);
        lv_label_set_text(cell.label, "");

        if (cell.adjacentMines > 0) {
            char text[12];
            snprintf(text, sizeof(text), "%d", cell.adjacentMines);
            lv_label_set_text(cell.label, text);
            lv_obj_set_style_text_color(cell.label, getNumberColor(cell.adjacentMines), 0);
        } else {
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dx == 0 && dy == 0) continue;
                    revealQueue_.push({x + dx, y + dy});
                }
            }
        }
    }

    if (revealQueue_.empty()) {
        checkWin();
        lv_timer_del(revealTimer_);
        revealTimer_ = nullptr;
    }
}

void processRevealStepAdapter(lv_timer_t* timer) {
    auto* self = static_cast<Minesweeper*>(lv_timer_get_user_data(timer));
    self->processRevealStep();
}

void Minesweeper::startRevealFrom(int x, int y) {
    if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) return;

    revealQueue_ = {};
    revealQueue_.push({x, y});

    if (revealTimer_) {
        lv_timer_del(revealTimer_);
    }

    revealTimer_ = lv_timer_create(processRevealStepAdapter, 30, nullptr);
    lv_timer_set_user_data(revealTimer_, this);
}

void Minesweeper::handleKey(uint32_t key) {
    if (!gameRunning_) return;

    auto &old = grid_[cursorY_][cursorX_];
    lv_obj_set_style_border_width(old.obj, 1, 0);
    lv_obj_set_style_border_color(old.obj, lv_color_make(128,128,128), 0);

    switch (key) {
        case LV_KEY_UP:
            cursorY_ = std::max(0, cursorY_ - 1);
            break;
        case LV_KEY_DOWN:
            cursorY_ = std::min(GRID_HEIGHT - 1, cursorY_ + 1);
            break;
        case LV_KEY_LEFT:
            cursorX_ = std::max(0, cursorX_ - 1);
            break;
        case LV_KEY_RIGHT:
            cursorX_ = std::min(GRID_WIDTH - 1, cursorX_ + 1);
            break;
        case LV_KEY_ENTER:
            {
                Cell& cell = grid_[cursorY_][cursorX_];
                if (cell.state == HIDDEN) {
                    toggleFlag(cursorX_, cursorY_);
                } else if (cell.state == FLAGGED) {
                    cell.state = HIDDEN;
                    totalFlags_--;
                    updateDisplay();
                    
                    if (firstClick_) {
                        firstClick_ = false;
                        placeMines();
                        calculateNumbers();
                    }
                    startRevealFrom(cursorX_, cursorY_);
                }
            }
    }

    auto &n = grid_[cursorY_][cursorX_];
    lv_obj_set_style_border_width(n.obj, 3, 0);
    lv_obj_set_style_border_color(n.obj, lv_color_make(0, 0, 255), 0);
}

void Minesweeper::createGameScreen() {
    const int SCREEN_W         = 320;
    const int TOP_OFFSET       = 120;
    const int CELL_BORDER      = 1;
    const int CONTAINER_BORDER = 2;
    const int PADDING          = 8;

    const int gridW = GRID_WIDTH  * CELL_SIZE;
    const int gridH = GRID_HEIGHT * CELL_SIZE;
    const int boardW = gridW + PADDING + CELL_BORDER*2;
    const int boardH = gridH + PADDING + CELL_BORDER*2;

    screen_ = createCleanObject(nullptr);
    lv_obj_set_style_bg_color(screen_, lv_color_make(50, 50, 50), 0);

    statusLabel_ = lv_label_create(screen_);
    applyCleanStyle(statusLabel_);
    lv_obj_set_pos(statusLabel_, 10, 10);
    lv_label_set_text(statusLabel_, "Minesweeper");
    lv_obj_set_style_text_font(statusLabel_, &lv_font_montserrat_20, 0);

    mineCountLabel_ = lv_label_create(screen_);
    applyCleanStyle(mineCountLabel_);
    lv_obj_set_pos(mineCountLabel_, 10, 40);
    lv_obj_set_style_text_font(mineCountLabel_, &lv_font_montserrat_20, 0);

    gameBoard_ = createCleanObject(screen_);
    lv_obj_set_size(gameBoard_, boardW, boardH);
    lv_obj_set_pos(gameBoard_, (SCREEN_W - boardW)/2, TOP_OFFSET);
    lv_obj_set_style_border_width(gameBoard_, CONTAINER_BORDER, 0);
    lv_obj_set_style_bg_color(gameBoard_, lv_color_make(128,128,128), 0);
    lv_obj_set_style_bg_opa(gameBoard_, LV_OPA_COVER, 0);

    const int inner = PADDING/2 + CELL_BORDER;

    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            Cell &c = grid_[y][x];
            c.obj = createCleanObject(gameBoard_);
            lv_obj_set_size(c.obj, CELL_SIZE - 1, CELL_SIZE - 1);
            lv_obj_set_pos(c.obj,
                inner + x*CELL_SIZE,
                inner + y*CELL_SIZE
            );

            lv_obj_set_style_bg_color(c.obj, lv_color_make(180,180,180), 0);
            lv_obj_set_style_bg_opa(c.obj, LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(c.obj, 1, 0);
            lv_obj_set_style_border_color(c.obj, lv_color_make(100,100,100), 0);
            lv_obj_set_style_radius(c.obj, 2, 0);

            c.label = lv_label_create(c.obj);
            applyCleanStyle(c.label);
            lv_label_set_text(c.label, "");
            lv_obj_center(c.label);
            lv_obj_set_style_text_font(c.label, &lv_font_montserrat_14, 0);

            c.hasMine = false;
            c.adjacentMines = 0;
            c.state = HIDDEN;
        }
    }

    cursorX_ = cursorY_ = 0;
    lv_obj_set_style_border_width(grid_[0][0].obj, 3, 0);
    lv_obj_set_style_border_color(grid_[0][0].obj, lv_color_make(0, 0, 255), 0);

    lv_obj_t* instr = lv_label_create(screen_);
    applyCleanStyle(instr);
    lv_label_set_text(instr,
        "Arrows: Move\n"
        "Enter: Flag\n"
        "Enter after flag: Reveal\n"
        "ESC: Exit"
    );
    lv_obj_align_to(instr, gameBoard_, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_style_text_color(instr, lv_color_make(64,64,64), 0);
    lv_obj_set_style_text_align(instr, LV_TEXT_ALIGN_CENTER, 0);

    lv_scr_load(screen_);
}



void Minesweeper::resetGame() {
    flagCount_ = 0;
    revealedCount_ = 0;
    totalFlags_ = 0;
    firstClick_ = true;
    cursorX_ = 0;
    cursorY_ = 0;
    
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            Cell& cell = grid_[y][x];
            cell.hasMine = false;
            cell.adjacentMines = 0;
            cell.state = HIDDEN;
            lv_label_set_text(cell.label, "");
            lv_obj_set_style_bg_color(cell.obj, lv_color_make(200, 200, 200), 0);
        }
    }
    
    updateDisplay();
}

void Minesweeper::placeMines() {
    std::vector<std::pair<int, int>> positions;
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (x != cursorX_ || y != cursorY_) {
                positions.push_back({x, y});
            }
        }
    }
    
    std::shuffle(positions.begin(), positions.end(), gen_);
    
    for (int i = 0; i < MINE_COUNT && i < positions.size(); i++) {
        int x = positions[i].first;
        int y = positions[i].second;
        grid_[y][x].hasMine = true;
    }
}

void Minesweeper::calculateNumbers() {
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (!grid_[y][x].hasMine) {
                int count = 0;
                
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        
                        int nx = x + dx;
                        int ny = y + dy;
                        
                        if (nx >= 0 && nx < GRID_WIDTH && 
                            ny >= 0 && ny < GRID_HEIGHT &&
                            grid_[ny][nx].hasMine) {
                            count++;
                        }
                    }
                }
                
                grid_[y][x].adjacentMines = count;
            }
        }
    }
}

void Minesweeper::revealCell(int x, int y) {
    if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) return;

    std::queue<std::pair<int, int>> toReveal;
    toReveal.push({x, y});

    while (!toReveal.empty()) {
        auto [cx, cy] = toReveal.front();
        toReveal.pop();

        if (cx < 0 || cx >= GRID_WIDTH || cy < 0 || cy >= GRID_HEIGHT)
            continue;

        Cell& cell = grid_[cy][cx];

        if (cell.state == REVEALED)
            continue;

        if (cell.state == FLAGGED) {
            cell.state = HIDDEN;
            totalFlags_--;
            lv_label_set_text(cell.label, "");
            lv_obj_set_style_bg_color(cell.obj, lv_color_make(180, 180, 180), 0);
            updateDisplay();
        }

        if (cell.state != HIDDEN)
            continue;

        cell.state = REVEALED;
        revealedCount_++;

        //lv_label_set_text(cell.label, "");
        //lv_obj_set_style_text_color(cell.label, lv_color_make(0, 0, 0), 0);
        //lv_obj_set_style_bg_color(cell.obj, lv_color_make(140, 140, 140), 0);

        if (cell.hasMine) {
            lv_label_set_text(cell.label, "*");
            lv_obj_set_style_bg_color(cell.obj, lv_color_make(0, 0, 255), 0);
            lv_obj_set_style_text_color(cell.label, lv_color_make(255, 255, 255), 0);
            revealAllMines();
            gameOver(false);
            return;
        }

        if (cell.adjacentMines > 0) {
            char text[12];
            snprintf(text, sizeof(text), "%d", cell.adjacentMines);
            lv_label_set_text(cell.label, text);
            lv_obj_set_style_text_color(cell.label, getNumberColor(cell.adjacentMines), 0);
        } else {
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dx == 0 && dy == 0) continue;
                    toReveal.push({cx + dx, cy + dy});
                }
            }
        }
    }

    checkWin();
}


void Minesweeper::toggleFlag(int x, int y) {
    Cell& cell = grid_[y][x];
    
    if (cell.state == REVEALED) return;
    
    if (cell.state == HIDDEN) {
        cell.state = FLAGGED;
        totalFlags_++;
        lv_label_set_text(cell.label, "F");
        lv_obj_set_style_text_color(cell.label, lv_color_make(0, 0, 255), 0);
        lv_obj_set_style_bg_color(cell.obj, lv_color_make(255, 200, 200), 0);
    } else if (cell.state == FLAGGED) {
        cell.state = HIDDEN;
        totalFlags_--;
        lv_label_set_text(cell.label, "");
        lv_obj_set_style_bg_color(cell.obj, lv_color_make(180, 180, 180), 0);
    }
    
    updateDisplay();
    
    if (totalFlags_ == MINE_COUNT) {
        bool allCorrect = true;
        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                if (grid_[y][x].state == FLAGGED && !grid_[y][x].hasMine) {
                    allCorrect = false;
                    break;
                }
            }
            if (!allCorrect) break;
        }
        if (allCorrect) {
            checkWin();
        }
    }
}

void Minesweeper::revealAllMines() {
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            Cell& cell = grid_[y][x];
            if (cell.hasMine && cell.state != FLAGGED) {
                lv_label_set_text(cell.label, "*");
                lv_obj_set_style_bg_color(cell.obj, lv_color_make(0, 0, 200), 0);
                lv_obj_set_style_text_color(cell.label, lv_color_make(255, 255, 255), 0);
            } else if (!cell.hasMine && cell.state == FLAGGED) {
                lv_label_set_text(cell.label, "X");
                lv_obj_set_style_bg_color(cell.obj, lv_color_make(255, 200, 200), 0);
            }
        }
    }
}

void Minesweeper::checkWin() {
    int safeCells = GRID_WIDTH * GRID_HEIGHT - MINE_COUNT;
    
    if (revealedCount_ == safeCells) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                Cell& cell = grid_[y][x];
                if (cell.hasMine) {
                    cell.state = FLAGGED;
                    lv_label_set_text(cell.label, "F");
                    lv_obj_set_style_text_color(cell.label, lv_color_make(0, 255, 0), 0);
                    lv_obj_set_style_bg_color(cell.obj, lv_color_make(200, 255, 200), 0);
                }
            }
        }
        gameOver(true);
    }
}

void Minesweeper::updateDisplay() {
    char mineText[50];
    snprintf(mineText, sizeof(mineText), "Mines: %d | Flags: %d", MINE_COUNT - totalFlags_, totalFlags_);
    lv_label_set_text(mineCountLabel_, mineText);
}

void Minesweeper::gameOver(bool win) {
    gameRunning_ = false;
    
    if (win) {
        lv_label_set_text(statusLabel_, "YOU WIN!");
        lv_obj_set_style_text_color(statusLabel_, lv_color_make(0, 200, 0), 0);
    } else {
        lv_label_set_text(statusLabel_, "GAME OVER!");
        lv_obj_set_style_text_color(statusLabel_, lv_color_make(0, 0, 200), 0);
    }
}

lv_color_t Minesweeper::getNumberColor(int number) {
    switch (number) {
        case 1: return lv_color_make(255, 0, 0);      // Blue
        case 2: return lv_color_make(0, 128, 0);      // Green
        case 3: return lv_color_make(0, 0, 255);      // Red
        case 4: return lv_color_make(128, 0, 0);      // Dark Blue
        case 5: return lv_color_make(0, 0, 128);      // Dark Red
        case 6: return lv_color_make(128, 128, 0);    // Cyan
        case 7: return lv_color_make(0, 0, 0);        // Black
        case 8: return lv_color_make(128, 128, 128);  // Gray
        default: return lv_color_make(0, 0, 0);
    }
}