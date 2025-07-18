# ESP Console

This project implements a small game console based on the ESP32‑S3 MCU and LVGL for the UI. Several games are included and a menu allows the user to start a selected game using hardware buttons.

## Overview

The firmware runs on an ESP32‑S3 board with a parallel LCD display. It uses LVGL for graphics and custom drivers for the display and buttons. Games register themselves at startup via the `GameRegistry` and are shown in the menu. When a game is launched, input is routed directly to it until the user exits back to the menu.

## Directory Structure

```
core/         - `Game` interface and `GameRegistry` implementation
hw_drivers/   - low level drivers for LCD and input GPIO
lvgl_app/     - LVGL initialization and tick/task handling
platform/     - input routing helper (InputRouter)
ui/           - LVGL helper utilities
screens/      - Menu and screen management
games/        - Individual game implementations
main/         - Application entry point and build glue
components/   - LVGL component (submodule)
```

### Folder Responsibilities

- **core** – defines the `Game` base class and singleton `GameRegistry` used for registering and creating games.
- **hw_drivers** – drivers for GPIO buttons, display controller (ILI9481) and hardware info helpers.
- **lvgl_app** – sets up LVGL, allocates buffers and connects LVGL with the custom display driver.
- **platform** – contains `InputRouter` which forwards button events from LVGL to the current screen or game.
- **screens** – UI screens such as the game selection menu and the `ScreenManager` that switches between menu and active game.
- **games** – implementations of games like Tetris, Snake, etc. Each game implements `Game` and registers itself via a global `RegisterXxx` struct.
- **ui** – small helpers for creating LVGL widgets without boilerplate.
- **main** – application entry (`app_main`) and component registration for ESP‑IDF.

## Building

The project targets **ESP‑IDF v5.4.1** as defined in `dependencies.lock`. Clone the repository with submodules and set up ESP‑IDF accordingly. Then run:

```bash
idf.py set-target esp32s3
idf.py build
```

## Flashing

Connect your ESP32‑S3 board and flash using:

```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

Replace `/dev/ttyUSB0` with your serial port. After flashing the console starts and the menu appears on the display.

## Usage

Navigate the menu with the hardware buttons (mapped to LVGL keypad keys) and press **ENTER** to launch a game. Press **ESC/BACKSPACE** while in a game to return to the menu.

Known limitations:

- Only basic debouncing is implemented for buttons.
- The graphics use a partial buffer which may limit performance on very complex screens.

## Diagrams

### Architecture

```plantuml
!include docs/diagrams/architecture.puml
```

### Game Launch Sequence

```plantuml
!include docs/diagrams/game_launch_sequence.puml
```

## Dependencies

- [ESP‑IDF 5.4.1](https://github.com/espressif/esp-idf)
- [LVGL](https://github.com/lvgl/lvgl) (included as submodule under `components/lvgl`)

