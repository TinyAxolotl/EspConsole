#include <stdint.h>
#include <stdio.h>
#include "esp_err.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_lcd_io_i80.h"
#include "hal/lcd_types.h"
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
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_commands.h"
#include "esp_check.h"
#include "esp_compiler.h"
#include "esp_cache.h"

#include "lvgl.h"
#include <demos/lv_demos.h>
#define TAG         "DIPLOM"

#define DISP_WIDTH   320
#define DISP_HEIGHT  480

#define FB_SIZE ((DISP_WIDTH * DISP_HEIGHT) / 10)

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
#define LCD_WR    20
#define LCD_CS    -1
#define LCD_RST   21

#define LCD_DATA_BUS_MASK (0xFFFF << 4)

typedef struct {
    esp_lcd_panel_t base;
    esp_lcd_panel_io_handle_t io;
    int reset_gpio_num;
    bool reset_level;
    int x_gap;
    int y_gap;
    uint8_t fb_bits_per_pixel;
    uint8_t madctl_val; // save current value of LCD_CMD_MADCTL register
    uint8_t colmod_val; // save current value of LCD_CMD_COLMOD register
} ili9481_panel_t;

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

bool transaction_done_cb(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *disp)
{
    lv_display_flush_ready(disp);

    return true;
}


void my_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *color_map) {
    if (NULL == disp || NULL == area || NULL == color_map) {
        ESP_LOGE(TAG, "Invalid parameters: disp = %p, area = %p, color_map = %p", disp, area, color_map);
        lv_display_flush_ready(disp);
        return;
    }
    
    esp_lcd_panel_handle_t panel_handle = lv_display_get_user_data(disp);

    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map);
}

void tick_inc(void *pvParameters) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        lv_tick_inc(10);
    }
}

// Helper structure, inherited from the base panel
typedef struct {
    esp_lcd_panel_t base;                     // base class
    esp_lcd_panel_io_handle_t io;             // handle for data/command transmission
    int reset_gpio;                           // GPIO for hardware reset, if used
    int x_gap;                                // X offset
    int y_gap;                                // Y offset
    // Additional internal states can be added, e.g., saving current MADCTL configuration
} panel_ili9481_t;

/***************************************************************************************************
 * Panel deletion function
 **************************************************************************************************/
static esp_err_t panel_ili9481_del(esp_lcd_panel_t *panel)
{
    panel_ili9481_t *ili = __containerof(panel, panel_ili9481_t, base);
    ESP_LOGI(TAG, "Deleting ILI9481 panel");

    // Free GPIO if used for reset
    if (ili->reset_gpio >= 0) {
        gpio_reset_pin(ili->reset_gpio);
    }
    free(ili);
    return ESP_OK;
}

/***************************************************************************************************
 * Hardware and/or software reset of the ILI9481 panel
 **************************************************************************************************/
static esp_err_t panel_ili9481_reset(esp_lcd_panel_t *panel)
{
    panel_ili9481_t *ili = __containerof(panel, panel_ili9481_t, base);
    ESP_LOGI(TAG, "Hardware reset ILI9481");

    // Perform hardware reset if reset GPIO is defined
    if (ili->reset_gpio >= 0) {
        gpio_set_level(ili->reset_gpio, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(ili->reset_gpio, 1);
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    // Additionally, perform SW reset by sending the command
    esp_lcd_panel_io_tx_param(ili->io, LCD_CMD_SWRESET, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(50));

    return ESP_OK;
}

/***************************************************************************************************
 * ILI9481 display initialization
 *
 * This sends the command sequence required for proper display startup.
 * The sequence may vary depending on the specific module requirements.
 **************************************************************************************************/
static esp_err_t panel_ili9481_init(esp_lcd_panel_t *panel)
{
    panel_ili9481_t *ili = __containerof(panel, panel_ili9481_t, base);
    ESP_LOGI(TAG, "Initializing ILI9481");

    // SW reset
    esp_lcd_panel_io_tx_param(ili->io, 0x01, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(120));

    // Exit sleep mode
    esp_lcd_panel_io_tx_param(ili->io, 0x11, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(120));

    // Set pixel format to 16-bit
    uint8_t pixel_fmt = 0x55; 
    esp_lcd_panel_io_tx_param(ili->io, 0x3A, &pixel_fmt, 1);

    // Exit invert mode (if needed)
    esp_lcd_panel_io_tx_param(ili->io, 0x20, NULL, 0);

    // MADCTL (orientation, BGR/XYZ)
    uint8_t madctl = 0x08; 
    esp_lcd_panel_io_tx_param(ili->io, 0x36, &madctl, 1);

    // Display on
    esp_lcd_panel_io_tx_param(ili->io, 0x29, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(50));

    return ESP_OK;
}

/***************************************************************************************************
 * Drawing a bitmap on the ILI9481 display
 *
 * x_start, y_start, x_end, y_end – drawing area coordinates (considering gap)
 * color_data – pointer to color data (format depends on panel settings)
 **************************************************************************************************/
static inline esp_err_t panel_ili9481_draw_bitmap(esp_lcd_panel_t *panel,
                                           int x_start, int y_start,
                                           int x_end, int y_end,
                                           const void *color_data)
{
    if (NULL == panel || NULL == color_data) {
        ESP_LOGE(TAG, "Invalid parameters: panel = %p, color_data = %p", panel, color_data);
        return ESP_ERR_INVALID_ARG;
    }

    panel_ili9481_t *ili = __containerof(panel, panel_ili9481_t, base);

    if (NULL == ili) {
        ESP_LOGE(TAG, "Failed to get ili9481 panel handle: %p\n", ili);

        return ESP_ERR_INVALID_ARG;
    }

    // Store original values for width and height calculation
    int orig_x_start = x_start;
    int orig_y_start = y_start;

    // Convert [x_start, x_end) to [x_start, x_end_incl], as the display requires inclusive bounds
    int x_end_incl = x_end - 1;
    int y_end_incl = y_end - 1;

    // Apply gaps
    x_start    += ili->x_gap;
    x_end_incl += ili->x_gap;
    y_start    += ili->y_gap;
    y_end_incl += ili->y_gap;

    uint8_t buf[4];

    // Set columns (CASET)
    buf[0] = (x_start >> 8) & 0xFF;
    buf[1] = x_start & 0xFF;
    buf[2] = (x_end_incl >> 8) & 0xFF;
    buf[3] = x_end_incl & 0xFF;
    esp_err_t err = esp_lcd_panel_io_tx_param(ili->io, LCD_CMD_CASET, buf, sizeof(buf));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "CASET failed (error %d)", err);
        return err;
    }

    // Set rows (RASET)
    buf[0] = (y_start >> 8) & 0xFF;
    buf[1] = y_start & 0xFF;
    buf[2] = (y_end_incl >> 8) & 0xFF;
    buf[3] = y_end_incl & 0xFF;
    err = esp_lcd_panel_io_tx_param(ili->io, LCD_CMD_RASET, buf, sizeof(buf));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "RASET failed (error %d)", err);
        return err;
    }

    // Calculate the number of pixels: width = (x_end - orig_x_start), height = (y_end - orig_y_start)
    int width  = x_end - orig_x_start;
    int height = y_end - orig_y_start;
    int pixel_count = width * height;

    // Send color data (16 bits per pixel)
    err = esp_lcd_panel_io_tx_color(ili->io, LCD_CMD_RAMWR, color_data, pixel_count * 2);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "RAMWR failed (error %d)", err);
        return err;
    }

    return ESP_OK;
}

/***************************************************************************************************
 * Invert display colors
 **************************************************************************************************/
static esp_err_t panel_ili9481_invert_color(esp_lcd_panel_t *panel, bool invert_color_data)
{
    panel_ili9481_t *ili = __containerof(panel, panel_ili9481_t, base);

    if (invert_color_data) {
        esp_lcd_panel_io_tx_param(ili->io, LCD_CMD_INVON, NULL, 0);
    } else {
        esp_lcd_panel_io_tx_param(ili->io, LCD_CMD_INVOFF, NULL, 0);
    }
    return ESP_OK;
}

/***************************************************************************************************
 * Mirror display horizontally/vertically
 *
 * Uses MADCTL command to set bits responsible for mirroring.
 * Bit values may need adjustment for the specific display.
 **************************************************************************************************/
static esp_err_t panel_ili9481_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    panel_ili9481_t *ili = __containerof(panel, panel_ili9481_t, base);
    uint8_t param = 0;

    // Assume:
    // - bit 0 controls horizontal mirroring
    // - bit 1 controls vertical mirroring
    if (mirror_x) {
        param |= (1 << 0);
    }
    if (mirror_y) {
        param |= (1 << 1);
    }
    esp_lcd_panel_io_tx_param(ili->io, LCD_CMD_MADCTL, &param, 1);
    return ESP_OK;
}

/***************************************************************************************************
 * Swap X and Y axes
 *
 * Usually done by setting the corresponding bit in MADCTL (e.g., bit for row/column exchange).
 **************************************************************************************************/
static esp_err_t panel_ili9481_swap_xy(esp_lcd_panel_t *panel, bool swap_axes)
{
    panel_ili9481_t *ili = __containerof(panel, panel_ili9481_t, base);
    uint8_t param = 0;

    // Assume bit 5 controls axis swapping
    if (swap_axes) {
        param |= (1 << 5);
    }
    esp_lcd_panel_io_tx_param(ili->io, LCD_CMD_MADCTL, &param, 1);
    return ESP_OK;
}

/***************************************************************************************************
 * Set gap offsets for X and Y axes
 **************************************************************************************************/
static esp_err_t panel_ili9481_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap)
{
    panel_ili9481_t *ili = __containerof(panel, panel_ili9481_t, base);
    ili->x_gap = x_gap;
    ili->y_gap = y_gap;
    return ESP_OK;
}

/***************************************************************************************************
 * Turn display on/off
 *
 * Typical display commands:
 *  - 0x29 for ON (DISPON)
 *  - 0x28 for OFF (DISPOFF)
 **************************************************************************************************/
static esp_err_t panel_ili9481_disp_on_off(esp_lcd_panel_t *panel, bool off)
{
    panel_ili9481_t *ili = __containerof(panel, panel_ili9481_t, base);

    if (off) {
        esp_lcd_panel_io_tx_param(ili->io, 0x28, NULL, 0);
    } else {
        esp_lcd_panel_io_tx_param(ili->io, 0x29, NULL, 0);
    }
    return ESP_OK;
}

/***************************************************************************************************
 * Enter/exit display sleep mode
 *
 * Typically:
 *  - 0x10 – sleep in
 *  - 0x11 – sleep out
 **************************************************************************************************/
static esp_err_t panel_ili9481_sleep(esp_lcd_panel_t *panel, bool sleep)
{
    panel_ili9481_t *ili = __containerof(panel, panel_ili9481_t, base);

    if (sleep) {
        esp_lcd_panel_io_tx_param(ili->io, LCD_CMD_SLPIN, NULL, 0);
        vTaskDelay(pdMS_TO_TICKS(120));
    } else {
        esp_lcd_panel_io_tx_param(ili->io, LCD_CMD_SLPOUT, NULL, 0);
        vTaskDelay(pdMS_TO_TICKS(120));
    }
    return ESP_OK;
}

/***************************************************************************************************
 * Create (initialize) a new ILI9481 panel
 *
 * io – I/O handle for communication with the display;
 * panel_dev_config – configuration structure (including offsets and reset GPIO);
 * ret_panel – pointer to return the created panel.
 **************************************************************************************************/
esp_err_t esp_lcd_new_panel_ili9481(const esp_lcd_panel_io_handle_t io,
                                    const esp_lcd_panel_dev_config_t *panel_dev_config,
                                    esp_lcd_panel_handle_t *ret_panel)
{
    panel_ili9481_t *ili = calloc(1, sizeof(panel_ili9481_t));
    if (!ili) {
        return ESP_ERR_NO_MEM;
    }

    ili->base.del         = panel_ili9481_del;
    ili->base.reset       = panel_ili9481_reset;
    ili->base.init        = panel_ili9481_init;
    ili->base.draw_bitmap = panel_ili9481_draw_bitmap;
    ili->base.invert_color= panel_ili9481_invert_color;
    ili->base.mirror      = panel_ili9481_mirror;
    ili->base.swap_xy     = panel_ili9481_swap_xy;
    ili->base.set_gap     = panel_ili9481_set_gap;
    ili->base.disp_on_off = panel_ili9481_disp_on_off;
    ili->base.disp_sleep  = panel_ili9481_sleep;

    ili->io = io;
    ili->x_gap = 0;
    ili->y_gap = 0;

    ili->reset_gpio = LCD_RST;

    *ret_panel = &(ili->base);
    ESP_LOGI(TAG, "ILI9481 panel created successfully");
    return ESP_OK;
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
        .pclk_hz = 11000000, // 10 MHz TODO: Move to KConfig. After approx 11 MHz image starts corrupting. Too much speed for ILI display.
        .trans_queue_depth = 10,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, io_handle));
}


void example_init_lcd_panel(esp_lcd_panel_io_handle_t io_handle, esp_lcd_panel_handle_t *panel)
{
    esp_lcd_panel_handle_t panel_handle = NULL;

    ESP_LOGI(TAG, "Install LCD driver of ili9481");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9481(io_handle, &panel_config, &panel_handle));

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    // Set inversion, x/y coordinate order, x/y mirror according to your LCD module spec
    // the gap is LCD panel specific, even panels with the same driver IC, can have different gap value
    //esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, false, true);
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
    
    // Configure control pins: RS, WR, CS, RST CS - GND now ((uint64_t)1 << LCD_CS) |
    io_conf.pin_bit_mask = ((uint64_t)1 << LCD_RS) | ((uint64_t)1 << LCD_WR) |
                            ((uint64_t)1 << LCD_RST);
    gpio_config(&io_conf);

    esp_lcd_panel_io_handle_t io_handle = NULL;
    example_init_i80_bus(&io_handle);
  

    esp_lcd_panel_handle_t panel_handle = NULL;
    example_init_lcd_panel(io_handle, &panel_handle);

    lv_init();


    static lv_color_t buf_1[FB_SIZE];
    static lv_color_t buf_2[FB_SIZE];

    lv_display_t * disp = lv_display_create(DISP_WIDTH, DISP_HEIGHT);
    if (NULL == panel_handle) {
        printf("WTF HANDLE IS NULL!!!\n");
    }
    lv_display_set_user_data(disp, panel_handle);
    lv_display_set_flush_cb(disp, my_flush_cb);

    lv_display_set_buffers(disp, buf_1, buf_2, sizeof(buf_1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = transaction_done_cb
    };

    esp_lcd_panel_io_register_event_callbacks(io_handle, &cbs, disp);
    lv_demo_benchmark();

    xTaskCreate(tick_inc, "TickInc", 2048, NULL, 1, NULL);
    uint32_t nextRun = 0;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(nextRun));
        nextRun = lv_timer_handler();
    }
}
