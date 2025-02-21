#include <stdio.h>
#include "esp_err.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "spi_flash_mmap.h"
#include "esp_heap_caps.h"
#include "esp_psram.h"
#include "esp_wifi.h"

#include "esp32s3/rom/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "soc/gpio_struct.h"
#include "esp_rom_sys.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"

#include "lvgl.h"

#define TAG         "DIPLOM"

#define DISP_WIDTH   320
#define DISP_HEIGHT  480

#define LCD_DB0   4
#define LCD_DB1   5
#define LCD_DB2   6
#define LCD_DB3   7
#define LCD_DB4   8
#define LCD_DB5   9
#define LCD_DB6   10
#define LCD_DB7   11
#define LCD_DB8   12
#define LCD_DB9   13
#define LCD_DB10  14
#define LCD_DB11  15
#define LCD_DB12  16
#define LCD_DB13  17
#define LCD_DB14  18
#define LCD_DB15  19

#define LCD_RS    35
#define LCD_WR    36
#define LCD_CS    37
#define LCD_RST   38

#define LCD_DATA_BUS_MASK (0xFFFF << 4)

void lcd_set_data_bus(uint16_t data)
{
    GPIO.out_w1tc = LCD_DATA_BUS_MASK;
    GPIO.out_w1ts = ((uint32_t)data << 4) & LCD_DATA_BUS_MASK;
}

void ili9481_send_command(uint8_t cmd)
{
    gpio_set_level(LCD_CS, 0);
    gpio_set_level(LCD_RS, 0); // CMD mode
    lcd_set_data_bus(cmd);
    gpio_set_level(LCD_WR, 0);
    esp_rom_delay_us(1);
    gpio_set_level(LCD_WR, 1);
    gpio_set_level(LCD_CS, 1);
}

/* TODO: Optimise this function. Send one bit might be useful, but 
         more useful would be to send a pointer + size, and continiously
         send data for (i = 0; i < data_len; i++)
         Possibly might be done during LVGL integration.
*/
void ili9481_send_data(uint16_t data)
{
    gpio_set_level(LCD_CS, 0);
    gpio_set_level(LCD_RS, 1);
    lcd_set_data_bus(data);
    gpio_set_level(LCD_WR, 0);
    esp_rom_delay_us(1);
    gpio_set_level(LCD_WR, 1);
    gpio_set_level(LCD_CS, 1);
}

void ili9481_send_s_data(uint16_t data)
{
    lcd_set_data_bus(data);
    gpio_set_level(LCD_WR, 0);
    esp_rom_delay_us(1);
    gpio_set_level(LCD_WR, 1);
}

void ili9481_init(void)
{
    // Hardware reset: RST LOW for 50 ms, RST HIGH for 50 ms
    gpio_set_level(LCD_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(50));
    gpio_set_level(LCD_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
    
    // Soft-reset also. Just to be sure :)
    ili9481_send_command(0x01);
    vTaskDelay(pdMS_TO_TICKS(50));
    
    // Exit sleep mode
    ili9481_send_command(0x11);
    vTaskDelay(pdMS_TO_TICKS(120));
    
    // Set pixel format: 0x3A + 0x55 for 16-bit mode. It's enabled by default, however also just to be sure.
    ili9481_send_command(0x3A);
    ili9481_send_data(0x55);
    
    // Set columns addresses (0–319)
    ili9481_send_command(0x2A);
    ili9481_send_data(0x00);       // First col high byte
    ili9481_send_data(0x00);       // First col low byte
    ili9481_send_data(0x01);       // Final col high byte (319 >> 8)
    ili9481_send_data(0x3F);       // Final col low byte (319 & 0xFF)
    
    // Set rows address (0–479)
    ili9481_send_command(0x2B);
    ili9481_send_data(0x00);       // First row high byte
    ili9481_send_data(0x00);       // First row low byte
    ili9481_send_data(0x01);       // Final row high byte (479 >> 8)
    ili9481_send_data(0xDF);       // Final row low byte (479 = 0x1DF)
    
    ili9481_send_command(0x20);    // Exit invert mode

    ili9481_send_command(0x36);
    ili9481_send_data(0b00000010); // set orientation. TODO: Debug it. Investigate available orientation options.

    // Enable display
    ili9481_send_command(0x29);
    vTaskDelay(pdMS_TO_TICKS(50));
}

// Test function to drww filled rect
// x, y – left corner coords, width, height – size, color – 16-bit color val
void ili9481_draw_filled_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    // Set columns
    ili9481_send_command(0x2A);
    ili9481_send_data(x >> 8);
    ili9481_send_data(x & 0xFF);
    ili9481_send_data((x + width - 1) >> 8);
    ili9481_send_data((x + width - 1) & 0xFF);
    
    // Set rows
    ili9481_send_command(0x2B);
    ili9481_send_data(y >> 8);
    ili9481_send_data(y & 0xFF);
    ili9481_send_data((y + height - 1) >> 8);
    ili9481_send_data((y + height - 1) & 0xFF);
    
    // Write to display memory cmd
    ili9481_send_command(0x2C);
    ili9481_send_data(color);

    ili9481_send_command(0x3C);
    gpio_set_level(LCD_CS, 0);
    gpio_set_level(LCD_RS, 1);
    // Fill everything with color
    uint32_t total_pixels = width * height;
    for (uint32_t i = 0; i < total_pixels; i++) {
        ili9481_send_s_data(color);
    }
}


/* BT is disabled in menuconfig */
int disable_wireless_peripherials()
{
	if (esp_wifi_stop() != ESP_OK) {
	    printf("Failed to stop Wi-Fi\n");

	    return ESP_FAIL;
	}

	if (esp_wifi_deinit() != ESP_OK) {
	    printf("Failed to deinit Wi-Fi\n");
		
		return ESP_FAIL;
	}

	return ESP_OK;
}

void my_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *color_map) {

    esp_lcd_panel_handle_t panel_handle = lv_display_get_user_data(disp);
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;

    ili9481_send_command(0x2A);
    ili9481_send_data((area->x1 >> 8) & 0xFF);
    ili9481_send_data(area->x1 & 0xFF);
    ili9481_send_data((area->x2 >> 8) & 0xFF);
    ili9481_send_data(area->x2 & 0xFF);

    ili9481_send_command(0x2B);
    ili9481_send_data((area->y1 >> 8) & 0xFF);
    ili9481_send_data(area->y1 & 0xFF);
    ili9481_send_data((area->y2 >> 8) & 0xFF);
    ili9481_send_data(area->y2 & 0xFF);

    ili9481_send_command(0x2C);
    ili9481_send_command(0x3C);
    gpio_set_level(LCD_CS, 0);
    gpio_set_level(LCD_RS, 1);

    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);

    lv_display_flush_ready(disp);
    printf("CB called and processed!\n");
}

void tick_inc(void *pvParameters) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        lv_tick_inc(10);
    }
}

void example_init_i80_bus(esp_lcd_panel_io_handle_t *io_handle)
{
    ESP_LOGI(TAG, "Initialize Intel 8080 bus");
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .dc_gpio_num = LCD_RS,
        .wr_gpio_num = LCD_WR,
        .data_gpio_nums = {
            LCD_DB0,
            LCD_DB1,
            LCD_DB2,
            LCD_DB3,
            LCD_DB4,
            LCD_DB5,
            LCD_DB6,
            LCD_DB7,
            LCD_DB8,
            LCD_DB9,
            LCD_DB10,
            LCD_DB11,
            LCD_DB12,
            LCD_DB13,
            LCD_DB14,
            LCD_DB15
        },
        .bus_width = 16,
        .max_transfer_bytes = 320 * 480 * sizeof(uint16_t),
        .dma_burst_size = 64,
    };
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = LCD_CS,
        .pclk_hz = 10000000,
        .trans_queue_depth = 10,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, io_handle));
}

void example_init_lcd_panel(esp_lcd_panel_io_handle_t io_handle, esp_lcd_panel_handle_t *panel)
{
    esp_lcd_panel_handle_t panel_handle = NULL;
#if CONFIG_EXAMPLE_LCD_I80_CONTROLLER_ST7789
    ESP_LOGI(TAG, "Install LCD driver of st7789");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    // Set inversion, x/y coordinate order, x/y mirror according to your LCD module spec
    // the gap is LCD panel specific, even panels with the same driver IC, can have different gap value
    esp_lcd_panel_invert_color(panel_handle, true);
    esp_lcd_panel_set_gap(panel_handle, 0, 20);
#elif CONFIG_EXAMPLE_LCD_I80_CONTROLLER_NT35510
    ESP_LOGI(TAG, "Install LCD driver of nt35510");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_nt35510(io_handle, &panel_config, &panel_handle));

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    // Set inversion, x/y coordinate order, x/y mirror according to your LCD module spec
    // the gap is LCD panel specific, even panels with the same driver IC, can have different gap value
    esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, true, false);
#elif CONFIG_EXAMPLE_LCD_I80_CONTROLLER_ILI9341
    // ILI9341 is NOT a distinct driver, but a special case of ST7789
    // (essential registers are identical). A few lines further down in this code,
    // it's shown how to issue additional device-specific commands.
    ESP_LOGI(TAG, "Install LCD driver of ili9341 (st7789 compatible)");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    // Set inversion, x/y coordinate order, x/y mirror according to your LCD module spec
    // the gap is LCD panel specific, even panels with the same driver IC, can have different gap value
    esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_invert_color(panel_handle, false);
    // ILI9341 is very similar to ST7789 and shares the same driver.
    // Anything unconventional (such as this custom gamma table) can
    // be issued here in user code and need not modify the driver.
    esp_lcd_panel_io_tx_param(io_handle, 0xF2, (uint8_t[]) {
        0
    }, 1); // 3Gamma function disable
    esp_lcd_panel_io_tx_param(io_handle, 0x26, (uint8_t[]) {
        1
    }, 1); // Gamma curve 1 selected
    esp_lcd_panel_io_tx_param(io_handle, 0xE0, (uint8_t[]) {          // Set positive gamma
        0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00
    }, 15);
    esp_lcd_panel_io_tx_param(io_handle, 0xE1, (uint8_t[]) {          // Set negative gamma
        0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F
    }, 15);
#endif
    *panel = panel_handle;
}

void app_main(void)
{
	disable_wireless_peripherials(); // Wifi && BT is not needed here..

    // Info about CPU. TODO: Translate to eng later. And move to separate function. Now it's required just for debug purposes.
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("Чип: ESP32, ядер: %d, ревизия: %d\n", chip_info.cores, chip_info.revision);

    size_t internal_ram = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    printf("Общий объём внутренней ОЗУ: %zu КБ\n", internal_ram / 1024);

    size_t psram_size = esp_psram_get_size();
    if (psram_size > 0) {
        printf("Общий объём PSRAM: %zu МБ\n", psram_size / (1024 * 1024));
    } else {
        printf("PSRAM не обнаружен\n");
    }

    esp_flash_t *chip_flash = esp_flash_default_chip;
    uint32_t flash_size;
    if (esp_flash_get_size(chip_flash, &flash_size) != ESP_OK) {
        printf("Не удалось получить размер флеш-памяти\n");
        return;
    }
    printf("Общий объём ПЗУ: %lu МБ\n", flash_size / (1024 * 1024));

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
    
    // Configure control pins: RS, WR, CS, RST
    io_conf.pin_bit_mask = ((uint64_t)1 << LCD_RS) | ((uint64_t)1 << LCD_WR) |
                           ((uint64_t)1 << LCD_CS) | ((uint64_t)1 << LCD_RST);
    gpio_config(&io_conf);
    
    // Display init
    ili9481_init();
    ili9481_draw_filled_rect(0, 0, DISP_WIDTH, DISP_HEIGHT, 0xF800);

    esp_lcd_panel_io_handle_t io_handle = NULL;
    example_init_i80_bus(&io_handle);

    // Draw black rectangle 100x100 px in pos x50, y50
    //ili9481_draw_filled_rect(50, 50, 100, 100, 0x0000);
    lv_init();

    static lv_color_t buf_1[DISP_WIDTH * 10];
    static lv_color_t buf_2[DISP_WIDTH * 10];
    printf("Ololo\n");
    lv_display_t * disp = lv_display_create(DISP_WIDTH, DISP_HEIGHT); /* Basic initialization with horizontal and vertical resolution in pixels */
    lv_display_set_flush_cb(disp, my_flush_cb); /* Set a flush callback to draw to the display */
    lv_display_set_buffers(disp, buf_1, buf_2, sizeof(buf_1), LV_DISPLAY_RENDER_MODE_PARTIAL); /* Set an initialized buffer */

    /* Change Active Screen's background color */
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x00FF00), LV_PART_MAIN);
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xffffff), LV_PART_MAIN);

    /* Create a spinner */
    lv_obj_t * spinner = lv_spinner_create(lv_screen_active());

    lv_spinner_set_anim_params(spinner, 1000, 60);

    lv_obj_set_size(spinner, 128, 128);

    lv_obj_align(spinner, LV_ALIGN_CENTER, 0, 0);

    xTaskCreate(tick_inc, "TickInc", 2048, NULL, 1, NULL);
    uint32_t nextRun = 0;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(nextRun));
        nextRun = lv_timer_handler();
    }
}
