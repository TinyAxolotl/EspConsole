#include <stdint.h>

#include "lvgl_init.h"
#include "app_config.h"
#include "esp_log.h"
#include "esp_freertos_hooks.h"
#include "gpio_driver.h"
#include "display_driver.h"


#define TAG "LVGL APPLICATION"

static esp_lcd_panel_io_handle_t lcd_io_handle = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;

static 
void IRAM_ATTR lv_tick_hook(void)
{
    lv_tick_inc(portTICK_PERIOD_MS);
}

IRAM_ATTR static
void timer_handler(void *pvParameters) {
    uint32_t next_run = 0;
    while(true) {
        if(next_run < 5) next_run = 5;
        vTaskDelay(pdMS_TO_TICKS(next_run));
        next_run = lv_timer_handler();
        //ESP_LOGD(TAG, "Timer handler called, next Run in: %ld ms", next_run);
    }
}

static
bool transaction_done_cb(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *display)
{
    lv_display_flush_ready(display);
    return true;
}

static
void my_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *color_map) {
    if (NULL == display || NULL == area || NULL == color_map) {
        ESP_LOGE(TAG, "Invalid parameters: display = %p, area = %p, color_map = %p", display, area, color_map);
        lv_display_flush_ready(display);
        return;
    }

    esp_lcd_panel_handle_t panel_handle = lv_display_get_user_data(display);

    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map);
}

esp_err_t init_lvgl(lv_display_t **display)
{
	esp_err_t err = ESP_OK;

    init_gpio();
    init_lcd_display(&panel_handle, &lcd_io_handle);

    lv_init();

    static lv_color_t buf_1[FB_SIZE];
    static lv_color_t buf_2[FB_SIZE];

    *display = lv_display_create(DISP_WIDTH, DISP_HEIGHT);

    lv_display_set_user_data(*display, panel_handle);
    lv_display_set_flush_cb(*display, my_flush_cb);

    // TODO: In case if will not enough performance, investigate another modes. Not partial, but LV_DISPLAY_RENDER_MODE_DIRECT?
    lv_display_set_buffers(*display, buf_1, buf_2, sizeof(buf_1), LV_DISPLAY_RENDER_MODE_PARTIAL);

    const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = transaction_done_cb
    };

    esp_lcd_panel_io_register_event_callbacks(lcd_io_handle, &cbs, *display);

    err = esp_register_freertos_tick_hook_for_cpu(lv_tick_hook, 1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register lv tick hook");
    }

    // TODO: Is 64kb stack & priority 15 enough?
    xTaskCreatePinnedToCore(timer_handler, "TimerHandler", 65535, NULL, 15, NULL, 1);

    return err;
}