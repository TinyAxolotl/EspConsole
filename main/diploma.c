#include <stdio.h>
#include "esp_err.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "spi_flash_mmap.h"
#include "esp_heap_caps.h"
#include "esp_psram.h"
#include "esp_wifi.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"

#define LCD_DB0   4
#define LCD_DB15  19

#define LCD_RS    35
#define LCD_WR    36
#define LCD_CS    37
#define LCD_RST   38

void lcd_set_data_bus(uint16_t data)
{
    for (int i = 0; i < 16; i++) {
        gpio_set_level(LCD_DB0 + i, (data >> i) & 0x01);
    }
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
    
    // Fill everything with color
    uint32_t total_pixels = width * height;
    for (uint32_t i = 0; i < total_pixels; i++) {
        ili9481_send_data(color);
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
    
    // Draw black rectangle 100x100 px in pos x50, y50
    ili9481_draw_filled_rect(50, 50, 100, 100, 0x0000);
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ili9481_draw_filled_rect(100, 100, 200, 200, 0xAEAE);
        vTaskDelay(pdMS_TO_TICKS(1000));
        ili9481_draw_filled_rect(100, 100, 200, 200, 0xFFFF);
        vTaskDelay(pdMS_TO_TICKS(1000));
        ili9481_draw_filled_rect(100, 100, 200, 200, 0x0000);
        vTaskDelay(pdMS_TO_TICKS(1000));
        ili9481_draw_filled_rect(100, 100, 200, 200, 0xFFFF);
    }
}
