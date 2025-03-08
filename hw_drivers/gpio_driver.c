#include "gpio_driver.h"
#include "app_config.h"
#include "driver/gpio.h"
#include "soc/gpio_struct.h"

void init_gpio(){
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
}
