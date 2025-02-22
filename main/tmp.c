    // alloc draw buffers used by LVGL
    uint32_t draw_buf_alloc_caps = 0;
    //draw_buf_alloc_caps |= MALLOC_CAP_SPIRAM;

    size_t draw_buffer_sz = 320 * (480/10) * sizeof(lv_color16_t);
    void *buf1 = esp_lcd_i80_alloc_draw_buffer(io_handle, draw_buffer_sz, draw_buf_alloc_caps);
    void *buf2 = esp_lcd_i80_alloc_draw_buffer(io_handle, draw_buffer_sz, draw_buf_alloc_caps);
    assert(buf1);
    assert(buf2);

    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(disp, buf1, buf2, draw_buffer_sz, LV_DISPLAY_RENDER_MODE_PARTIAL); /* Set an initialized buffer */