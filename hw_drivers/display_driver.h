#pragma once

#include "app_config.h"

#include "driver/gpio.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_commands.h"

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

// Helper structure, inherited from the base panel
typedef struct {
    esp_lcd_panel_t base;                     // base class
    esp_lcd_panel_io_handle_t io;             // handle for data/command transmission
    int reset_gpio;                           // GPIO for hardware reset, if used
    int x_gap;                                // X offset
    int y_gap;                                // Y offset
    // Additional internal states can be added, e.g., saving current MADCTL configuration. TODO: Investigate if needed in future (?)
} panel_ili9481_t;


/***************************************************************************************************
 * Panel deletion function
 **************************************************************************************************/
esp_err_t panel_ili9481_del(esp_lcd_panel_t *panel);


/***************************************************************************************************
 * Hardware and/or software reset of the ILI9481 panel
 **************************************************************************************************/
esp_err_t panel_ili9481_reset(esp_lcd_panel_t *panel);


/***************************************************************************************************
 * ILI9481 display initialization
 *
 * This sends the command sequence required for proper display startup.
 * The sequence may vary depending on the specific module requirements.
 **************************************************************************************************/
esp_err_t panel_ili9481_init(esp_lcd_panel_t *panel);


/***************************************************************************************************
 * Drawing a bitmap on the ILI9481 display
 *
 * x_start, y_start, x_end, y_end – drawing area coordinates (considering gap)
 * color_data – pointer to color data (format depends on panel settings)
 **************************************************************************************************/
esp_err_t panel_ili9481_draw_bitmap(esp_lcd_panel_t *panel,
                                   int x_start, int y_start,
                                   int x_end, int y_end,
                                   const void *color_data);


/***************************************************************************************************
 * Invert display colors
 **************************************************************************************************/
esp_err_t panel_ili9481_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);


/***************************************************************************************************
 * Mirror display horizontally/vertically
 *
 * Uses MADCTL command to set bits responsible for mirroring.
 * Bit values may need adjustment for the specific display.
 **************************************************************************************************/
esp_err_t panel_ili9481_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);


/***************************************************************************************************
 * Swap X and Y axes
 *
 * Usually done by setting the corresponding bit in MADCTL (e.g., bit for row/column exchange).
 **************************************************************************************************/
esp_err_t panel_ili9481_swap_xy(esp_lcd_panel_t *panel, bool swap_axes);


/***************************************************************************************************
 * Set gap offsets for X and Y axes
 **************************************************************************************************/
esp_err_t panel_ili9481_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap);


/***************************************************************************************************
 * Turn display on/off
 *
 * Typical display commands:
 *  - 0x29 for ON (DISPON)
 *  - 0x28 for OFF (DISPOFF)
 **************************************************************************************************/
esp_err_t panel_ili9481_disp_on_off(esp_lcd_panel_t *panel, bool off);


/***************************************************************************************************
 * Enter/exit display sleep mode
 *
 * Typically:
 *  - 0x10 – sleep in
 *  - 0x11 – sleep out
 **************************************************************************************************/
esp_err_t panel_ili9481_sleep(esp_lcd_panel_t *panel, bool sleep);


/***************************************************************************************************
 * Create (initialize) a new ILI9481 panel
 *
 * io – I/O handle for communication with the display;
 * panel_dev_config – configuration structure (including offsets and reset GPIO);
 * ret_panel – pointer to return the created panel.
 **************************************************************************************************/
esp_err_t esp_lcd_new_panel_ili9481(const esp_lcd_panel_io_handle_t io,
                                    const esp_lcd_panel_dev_config_t *panel_dev_config,
                                    esp_lcd_panel_handle_t *ret_panel);


esp_err_t init_lcd_display(esp_lcd_panel_handle_t *panel, esp_lcd_panel_io_handle_t *io_handle);
