#include "gpio_driver.h"
#include "app_config.h"
#include "core/lv_group.h"
#include "driver/gpio.h"
#include "soc/gpio_struct.h"
#include <stdint.h>
#include <stdio.h>

// pins: 1, 2, 3, 38, 41, 42, 47, 48
// connected: 47, 48, 38, 41, 42

static
void init_input_pins() {
    gpio_config_t io_conf = {0};

    // Маска пинов 1, 2, 3, 38, 39, 40, 41, 42, 45, 47, 48
    uint64_t pins_mask = (1ULL << PREV_BTN) | (1ULL << NEXT_BTN) | (1ULL << ACCEPT_BTN) |
                         (1ULL << DECREASE_BTN) | (1ULL << INCREASE_BTN) | (1ULL << ESC_BTN);

    io_conf.pin_bit_mask = pins_mask;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&io_conf);
}

void init_gpio() {
    gpio_config_t io_conf = {0};
    uint64_t data_pins_mask = 0;

    for (int i = LCD_DB0; i <= LCD_DB15; i++) {
        data_pins_mask |= ((uint64_t)1 << i);
    }

    io_conf.pin_bit_mask = data_pins_mask;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = 0;
    io_conf.pull_down_en = 0;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);
    
    // Configure control pins: RS, WR, CS, RST CS - GND now prev: ((uint64_t)1 << LCD_CS) |
    io_conf.pin_bit_mask = ((uint64_t)1 << LCD_RS) | ((uint64_t)1 << LCD_WR) |
                            ((uint64_t)1 << LCD_RST);
    gpio_config(&io_conf);

    init_input_pins();
}

int read_button() {
    if (!gpio_get_level(PREV_BTN)) {
        printf("PREV_BTN pressed\n");
        return LV_KEY_LEFT;
    }
    if (!gpio_get_level(NEXT_BTN)) {
        printf("NEXT_BTN pressed\n");
        return LV_KEY_RIGHT;
    }
    if (!gpio_get_level(DECREASE_BTN)) {
        printf("DECREASE_BTN pressed\n");
        return LV_KEY_DOWN;
    }
    if (!gpio_get_level(INCREASE_BTN)) {
        printf("INCREASE_BTN pressed\n");
        return LV_KEY_UP;
    }
    if (!gpio_get_level(ACCEPT_BTN)) {
        printf("ACCEPT_BTN pressed\n");
        return LV_KEY_ENTER;
    }
    if (!gpio_get_level(ESC_BTN)) {
        printf("ESC_BTN pressed\n");

        return LV_KEY_BACKSPACE;
    }

    return -1;
}
