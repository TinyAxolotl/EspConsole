#include <stdint.h>
#include <stdio.h>

#include "lvgl_init.h"
#include "app_config.h"
#include "esp_log.h"
#include "esp_freertos_hooks.h"
#include "gpio_driver.h"
#include "display_driver.h"


static const char *TAG = "lvgl_init";
void (*handle_input_event)(uint32_t key) = NULL;

static esp_lcd_panel_io_handle_t lcd_io_handle = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;

static void button_read(lv_indev_t * indev, lv_indev_data_t * data)
{
    static uint32_t last_key = 0;
    int key = read_button();

    if (key >= 0) {
        last_key = key;
        data->state = LV_INDEV_STATE_PRESSED;
        ESP_LOGI(TAG, "Button read: %d (pressed)", key);
        
        if (handle_input_event) {
            handle_input_event((uint32_t)key);
        } else {
            ESP_LOGE(TAG, "handle_input_event is NULL in button_read");
        }
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }

    data->key = last_key;
}

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

// PC -> 0xFFFF1234
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

    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(indev, button_read);

    return err;
}