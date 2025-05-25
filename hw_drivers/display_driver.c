#include "app_config.h"
#include "display_driver.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"

#define TAG "DISPLAY DRIVER"

esp_err_t panel_ili9481_del(esp_lcd_panel_t *panel)
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

esp_err_t panel_ili9481_reset(esp_lcd_panel_t *panel)
{
    panel_ili9481_t *ili = __containerof(panel, panel_ili9481_t, base);
    ESP_LOGI(TAG, "Hardware reset ILI9481");

    // Perform hardware reset if reset GPIO is defined TODO: Is it needed? Maybe bullshit?
    if (ili->reset_gpio >= 0) {
        gpio_set_level(ili->reset_gpio, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(ili->reset_gpio, 1);
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    // Additionally, perform SW reset by sending the command TODO: Is it needed? Maybe bullshit?
    esp_lcd_panel_io_tx_param(ili->io, LCD_CMD_SWRESET, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(50));

    return ESP_OK;
}

// TODO: Take a look on available init on github if available, just curious how others inits it
esp_err_t panel_ili9481_init(esp_lcd_panel_t *panel)
{
    panel_ili9481_t *ili = __containerof(panel, panel_ili9481_t, base);
    ESP_LOGI(TAG, "Initializing ILI9481");

    // SW reset
    esp_lcd_panel_io_tx_param(ili->io, LCD_CMD_SWRESET, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(120));
    ESP_LOGV(TAG, "Software reset performed");

    // Exit sleep mode
    esp_lcd_panel_io_tx_param(ili->io, LCD_CMD_SLPOUT, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(120));
    ESP_LOGV(TAG, "Exit sleep mode");

    // Set pixel format to 16-bit
    uint8_t pixel_fmt = 0x55; 
    esp_lcd_panel_io_tx_param(ili->io, LCD_CMD_COLMOD, &pixel_fmt, 1);
    ESP_LOGV(TAG, "16-bit per pixel configured");

    // Exit invert mode
    esp_lcd_panel_io_tx_param(ili->io, LCD_CMD_INVOFF, NULL, 0);
    ESP_LOGV(TAG, "Exited inverted mode");

    // MADCTL (orientation, BGR/XYZ)
    uint8_t madctl = 0x08; 
    esp_lcd_panel_io_tx_param(ili->io, LCD_CMD_MADCTL, &madctl, 1);
    ESP_LOGV(TAG, "Orientation configured");

    // Display on
    esp_lcd_panel_io_tx_param(ili->io, LCD_CMD_DISPON, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(50));
    ESP_LOGV(TAG, "Display enabled");

    ESP_LOGI(TAG, "ILI9481 configured");

    return ESP_OK;
}

esp_err_t panel_ili9481_draw_bitmap(esp_lcd_panel_t *panel,
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

    // TODO: Refactor below lines in future, should be more simple approach. 

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

    // Send color data
    err = esp_lcd_panel_io_tx_color(ili->io, LCD_CMD_RAMWR, color_data, pixel_count * 2);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "RAMWR failed (error %d)", err);
        return err;
    }

    return ESP_OK;
}

esp_err_t panel_ili9481_invert_color(esp_lcd_panel_t *panel, bool invert_color_data)
{
    panel_ili9481_t *ili = __containerof(panel, panel_ili9481_t, base);

    if (invert_color_data) {
        esp_lcd_panel_io_tx_param(ili->io, LCD_CMD_INVON, NULL, 0);
    } else {
        esp_lcd_panel_io_tx_param(ili->io, LCD_CMD_INVOFF, NULL, 0);
    }
    return ESP_OK;
}

esp_err_t panel_ili9481_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
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

esp_err_t panel_ili9481_swap_xy(esp_lcd_panel_t *panel, bool swap_axes)
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

esp_err_t panel_ili9481_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap)
{
    panel_ili9481_t *ili = __containerof(panel, panel_ili9481_t, base);
    ili->x_gap = x_gap;
    ili->y_gap = y_gap;
    return ESP_OK;
}

esp_err_t panel_ili9481_disp_on_off(esp_lcd_panel_t *panel, bool off)
{
    panel_ili9481_t *ili = __containerof(panel, panel_ili9481_t, base);

    if (off) {
        esp_lcd_panel_io_tx_param(ili->io, LCD_CMD_DISPOFF, NULL, 0);
    } else {
        esp_lcd_panel_io_tx_param(ili->io, LCD_CMD_DISPON, NULL, 0);
    }
    return ESP_OK;
}

esp_err_t panel_ili9481_sleep(esp_lcd_panel_t *panel, bool sleep)
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

static
esp_err_t init_i80_bus(esp_lcd_panel_io_handle_t *io_handle)
{
    ESP_LOGI(TAG, "Initialization Intel 8080 bus");

    esp_err_t err = ESP_OK;
    esp_lcd_i80_bus_handle_t i80_bus_handle = NULL;
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
        .bus_width = DISP_BUS_WIDTH,
        .max_transfer_bytes = DISP_WIDTH * DISP_HEIGHT * sizeof(uint16_t),
        .dma_burst_size = DMA_BURST_SIZE,
    };

    err = esp_lcd_new_i80_bus(&bus_config, &i80_bus_handle);
    if (ESP_OK != err) {
        ESP_LOGE(TAG, "Failed to create new i80 bus handle. Error: %s", esp_err_to_name(err));

        return err;
    }

    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = LCD_CS,
        .pclk_hz = LCD_FREQUENCY_HZ, // 10 MHz TODO: Move to KConfig. After approx 11 MHz image starts corrupting. Too much speed for ILI display.
        .trans_queue_depth = 10,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
        .lcd_cmd_bits = DISP_BUS_WIDTH/2,
        .lcd_param_bits = DISP_BUS_WIDTH/2,
    };

    err = esp_lcd_new_panel_io_i80(i80_bus_handle, &io_config, io_handle);
    if (ESP_OK != err) {
        ESP_LOGE(TAG, "Failed to create LCD panel IO. Error: %s", esp_err_to_name(err));

        return err;
    }

    return err;
}

esp_err_t init_lcd_display(esp_lcd_panel_handle_t *panel, esp_lcd_panel_io_handle_t *io_handle)
{
    esp_err_t err = ESP_OK;

    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_io_handle_t lcd_io_handle = NULL;
    init_i80_bus(&lcd_io_handle);

    ESP_LOGI(TAG, "Install LCD driver of ili9481");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = BITS_PER_PIXEL,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9481(lcd_io_handle, &panel_config, &panel_handle));

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);

    // TODO: Test couple of mirror & swap options. Is it make sense? Maybe it might be easier to mirror via lvgl if needed?

    //esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, false, true);
    *panel = panel_handle;
    *io_handle = lcd_io_handle;

    return err;
}
