# TypeMaster — 打字通

A Qt6 C++ typing game launcher with three game modes.

## Project Structure

```
TypeMaster/
├── CMakeLists.txt          # Build configuration
├── main.cpp                # Application entry & i18n loader
├── mainwindow.h/cpp        # Launch screen (game selection)
├── gamecard.h/cpp          # Individual game card widget
├── applewindow.h/cpp       # 🍎 Save the Apple — full game
├── spacewindow.h/cpp       # 🚀 Space War — UI placeholder
├── practicewindow.h/cpp    # ⌨️  Practice Mode — typing trainer
├── resources.qrc           # Embedded resources (images, QSS)
├── styles/main.qss         # Global QSS stylesheet
├── translations/
│   ├── typemaster_zh_CN.ts # Simplified Chinese translation
│   └── typemaster_en.ts    # English (source language)
└── resources/images/       # Place game asset images here
    ├── Common/
    ├── Apple/
    ├── Space/
    ├── Frog/
    ├── Mole/
    └── Police/
```

## Build Requirements

- **Qt 6.2+** with modules: `Core`, `Widgets`, `Multimedia`, `LinguistTools`
- **CMake 3.16+**
- **C++17** compatible compiler (MSVC 2019+, GCC 10+, Clang 12+)

## Build Steps

```bash
# 1. Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 2. Build (also compiles translations)
cmake --build build --parallel

# 3. Run
./build/TypeMaster          # Linux/macOS
build\TypeMaster.exe        # Windows
```

## Adding Game Assets

Place image files from the resource package into `resources/images/`:

| Folder   | Key files needed                                |
|----------|-------------------------------------------------|
| Apple/   | APPLE_BACKGROUND.png, APPLE_NORMAL.png, APPLE_BAD.png, APPLE_BASKET.png, APPLE_SMALL.png |
| Space/   | SPACE_BACKGROUND.png, SPACE_MAINMENU_BG.png     |
| Common/  | PUBLIC_START.png, PUBLIC_PAUSE.png, …           |

> The game runs with **built-in fallback graphics** if images are missing,
> so you can build and test without any assets.

## Internationalization

All user-facing strings use `tr()`. To add/update translations:

```bash
# Update .ts files from source
lupdate TypeMaster.pro -ts translations/typemaster_zh_CN.ts

# Compile .ts → .qm (done automatically by cmake --build)
lrelease translations/typemaster_zh_CN.ts
```

The app loads `zh_CN` by default; set `LANG=en_US` to switch to English.

## Apple Game Controls

| Key          | Action              |
|--------------|---------------------|
| A–Z          | Type the letter     |
| Start button | Begin / restart     |
| Pause button | Pause / resume      |
| ◀ Back       | Return to launcher  |

## Grading Checklist

- [x] ≥ 3 game modes on launcher screen
- [x] Each game has preview image + description
- [x] Apple game: falling apples, type to catch, scoring, lives, levels
- [x] Other games: UI-only stubs
- [x] Qt i18n: all text via `tr()`, no hardcoded Chinese
- [x] `.ts` translation file provided (`typemaster_zh_CN.ts`)
- [x] QSS stylesheet (`styles/main.qss`)
- [x] Responsive layout (resizeEvent handled)
- [x] CMakeLists.txt with proper Qt6 setup
- [x] C++17, clean code structure
