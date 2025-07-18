# Game Framework Overview

This document describes the common classes introduced to reduce code
duplication across games. They provide a small dependency injection
setup and RAII helpers for LVGL objects and timers.

## Classes

- **GameUI** – wrapper around basic LVGL operations. It can create a
  screen object via `createScreen()` and provides helpers like
  `createLabel()` and `applyCleanStyle()`.

- **TimerManager** – manages `lv_timer_t` instances. Timers created with
  `create()` are stored and automatically cleaned up when the manager is
  destroyed or `clear()` is called.

- **GameContext** – simple container that bundles references to
  `GameUI`, `TimerManager` and the global `InputRouter`. Games receive it
  through their constructor so they do not rely on globals.

- **BaseGame** – new abstract base class implementing the `Game`
  interface. It owns a screen and an update timer through the injected
  context. Derived games override `onStart()`, `onUpdate()` and
  `onInput()` instead of dealing with LVGL setup manually.

See `docs/diagrams/game_framework.puml` for a UML style overview.
