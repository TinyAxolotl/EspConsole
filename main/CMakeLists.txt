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
        "../ui/lvgl_helper.cpp"
        "../games/flappy_bird/FlappyBird.cpp"
        "../games/flappy_bird/assets/bird.c"
        "../games/tower_bloxx/assets/base.c"
        "../games/tower_bloxx/assets/blue_centre.c"
        "../games/tower_bloxx/assets/red_centre.c"
        "../games/tower_bloxx/assets/green_centre.c"
        "../games/tower_bloxx/assets/purple_centre.c"
        "../games/tetris/Tetris.cpp"
        "../games/arkanoid/Arkanoid.cpp"
        "../games/racing/Racing.cpp"
        "../games/snake/Snake.cpp"
        "../games/game2048/Game2048.cpp"
        "../games/minesweeper/Minesweeper.cpp"
        "../games/tower_bloxx/TowerBloxx.cpp"
    INCLUDE_DIRS
        "."
        "../hw_drivers"
        "../lvgl_app"
        "../core"
        "../platform"
        "../screens"
        "../ui"
        "../games/"
        "../games/flappy_bird"
        "../games/flappy_bird/assets"
        "../games/tower_bloxx/assets"
        "../games/tetris"
        "../games/arkanoid"
        "../games/racing"
        "../games/snake"
        "../games/game2048"
        "../games/minesweeper"
        "../games/tower_bloxx"
)

set(CONFIG_ESP_WIFI_ENABLED n CACHE INTERNAL "Disable WiFi")
