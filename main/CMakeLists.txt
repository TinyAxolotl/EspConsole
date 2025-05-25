idf_component_register(
    SRCS
        "diploma.c"
        "../hw_drivers/display_driver.c"
        "../hw_drivers/gpio_driver.c"
        "../hw_drivers/hardware_info.c"
        "../lvgl_app/lvgl_init.c"
        "app.cpp"
        "../platform/InputRouter.cpp"
        "../core/GameRegistry.cpp"
        "../screens/MenuScreen.cpp"
        "../screens/ScreenManager.cpp"
        "../games/simple_catcher/SimpleCatcher.cpp"
    INCLUDE_DIRS
        "."
        "../hw_drivers"
        "../lvgl_app"
        "../core"
        "../platform"
        "../screens"
        "../games/"
        "../games/simple_catcher"
)

set(CONFIG_ESP_WIFI_ENABLED n CACHE INTERNAL "Disable WiFi")
