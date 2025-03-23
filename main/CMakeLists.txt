idf_component_register(SRCS "diploma.c" "../hw_drivers/display_driver.c" "../hw_drivers/gpio_driver.c" "../lvgl_app/lvgl_init.c" "../hw_drivers/hardware_info.c" "logic_interface.cpp" "../core/GameManager.cpp" "../lvgl_app/ui/main_menu.cpp"
                    INCLUDE_DIRS "." "../hw_drivers" "../lvgl_app" ../lvgl_app/ui "../core")
set(CONFIG_ESP_WIFI_ENABLED n CACHE INTERNAL "Disable WiFi")
