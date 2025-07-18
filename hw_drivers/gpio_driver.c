#include "gpio_driver.h"
#include "app_config.h"
#include "core/lv_group.h"
#include "driver/gpio.h"
#include "soc/gpio_struct.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdio.h>
#include "esp_log.h"

static const char *TAG = "MyComponent";

typedef struct {
    int pin;
    int lv_key;
    uint8_t current_state;
    uint8_t previous_state;
    uint32_t last_press_time;
    const char* name;
} button_state_t;

static button_state_t buttons[] = {
    {INCREASE_BTN, LV_KEY_LEFT,      1, 1, 0, "PREV_BTN"},
    {DECREASE_BTN, LV_KEY_RIGHT,     1, 1, 0, "NEXT_BTN"},
    {NEXT_BTN,     LV_KEY_DOWN,      1, 1, 0, "DECREASE_BTN"},
    {PREV_BTN,     LV_KEY_UP,        1, 1, 0, "INCREASE_BTN"},
    {ACCEPT_BTN,   LV_KEY_ENTER,     1, 1, 0, "ACCEPT_BTN"},
    {ESC_BTN,      LV_KEY_BACKSPACE, 1, 1, 0, "ESC_BTN"}
};

#define BUTTON_COUNT (sizeof(buttons) / sizeof(buttons[0]))
#define DEBOUNCE_TIME_MS 50

static
void init_input_pins() {
    gpio_config_t io_conf = {0};

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
    
    // Configure control pins: RS, WR, RST
    io_conf.pin_bit_mask = ((uint64_t)1 << LCD_RS) | ((uint64_t)1 << LCD_WR) |
                            ((uint64_t)1 << LCD_RST);
    gpio_config(&io_conf);

    init_input_pins();
    
    ESP_LOGI(TAG, "GPIO initialized with debouncing");
}

int read_button() {
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    for (int i = 0; i < BUTTON_COUNT; i++) {
        button_state_t* btn = &buttons[i];
        
        btn->current_state = gpio_get_level(btn->pin);
        
        if (btn->previous_state == 1 && btn->current_state == 0) {
            if (current_time - btn->last_press_time >= DEBOUNCE_TIME_MS) {
                btn->last_press_time = current_time;
                btn->previous_state = btn->current_state;
                
                ESP_LOGI(TAG, "%s pressed (debounced)", btn->name);
                return btn->lv_key;
            }
        } else if (btn->previous_state == 0 && btn->current_state == 1) {
            btn->previous_state = btn->current_state;
        }
    }

    return -1;
}

// pins: 1, 2, 3, 38, 41, 42, 47, 48
// connected: 47, 48, 38, 41, 42
