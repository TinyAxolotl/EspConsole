#include "hardware_info.h"

#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"
#include "esp_psram.h"
#include "esp_log.h"

#define TAG "HARDWARE INFO"

void print_hardware_info() {
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    size_t psram_size;
    size_t internal_ram;
	esp_flash_t *chip_flash = NULL;

    esp_chip_info(&chip_info);
    ESP_LOGI(TAG, "Chip: ESP32, cores: %d, revision: %d\n", chip_info.cores, chip_info.revision);

    internal_ram = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    ESP_LOGI(TAG, "Internal RAM: %zu KB\n", internal_ram / 1024);

    psram_size = esp_psram_get_size();
    if (psram_size > 0) {
        ESP_LOGI(TAG, "PSRAM size: %zu MB\n", psram_size / (1024 * 1024));
    } else {
        ESP_LOGI(TAG, "PSRAM was not detected\n");
    }

    chip_flash = esp_flash_default_chip;
    
    if (esp_flash_get_size(chip_flash, &flash_size) != ESP_OK) {
        ESP_LOGI(TAG, "Failed to get flash size\n");
        return;
    }
    ESP_LOGI(TAG, "Flash size: %lu MB\n", flash_size / (1024 * 1024));
}
