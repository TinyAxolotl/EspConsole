#include "esp_log.h"
#include "lvgl.h"
#include "app_config.h"
#include "lvgl_init.h"
#include "hardware_info.h"
#include "logic_interface.h"

#define TAG         "DIPLOM"

void app_main(void)
{

    lv_display_t *display = NULL;

    print_hardware_info();

    init_lvgl(&display);

    while (1) {
           vTaskDelay(pdMS_TO_TICKS(100));
    }
}
