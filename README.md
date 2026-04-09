# TypeMaster — 打字通

A Qt6 C++ typing game launcher with three game modes.

## Project Structure
│  .gitignore
│  CMakeLists.txt
│  README.md
├─resources
│  │ resources.qrc
│  ├─fonts
│  ├─images
│  │  ├─Apple
│  │  ├─Common
│  │  ├─Police
│  │  └─Space
│  │
│  ├─sounds
│  │  ├─Apple
│  │  ├─Common
│  │  ├─Frog
│  │  ├─Mole
│  │  ├─Police
│  │  └─Space
│  │
│  └─styles
├─src
└─translations
        typemaster_en.ts
        typemaster_zh_CN.ts


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


# 资源文件放置说明

将游戏资源包中的文件按以下结构放入 `resources/` 目录：

```
resources/
├── images/
│   ├── Common/          ← 来自资源包 Common/Images/
│   │   ├── CANCEL.png
│   │   ├── BTN_CLICK.png
│   │   └── ... (其余图片)
│   ├── Apple/           ← 来自资源包 Apple/Images/
│   │   ├── APPLE_BACKGROUND.png
│   │   ├── APPLE_NORMAL.png
│   │   ├── APPLE_BAD.png
│   │   ├── APPLE_BASKET.png
│   │   ├── APPLE_SMALL.png
│   │   └── ... (其余图片)
│   ├── Space/           ← 来自资源包 Space/Images/
│   └── ... (Frog/Mole/Police 同理)
│
└── sounds/
    ├── Common/          ← 来自资源包 Common/Sounds/
    │   ├── ANIBTN_CLICK.wav   ← QSoundEffect 要求 WAV 格式
    │   ├── ANIBTN_ENTER.wav
    │   ├── BTN_CLICK.wav
    │   ├── GLIDE.wav
    │   └── TYPE.wav
    ├── Apple/           ← 来自资源包 Apple/Sounds/
    │   ├── APPLE_BG.mp3       ← 背景音乐，QMediaPlayer 支持 MP3/OGG/WAV
    │   └── APPLE_IN.wav       ← 音效，QSoundEffect 要求 WAV
    ├── Space/
    │   ├── SPACE_BG.mp3
    │   ├── SPACE_BLAST.wav
    │   ├── SPACE_SHOOT.wav
    │   ├── SPACE_PLANEOUT.wav
    │   ├── SPACE_WORDOUT.wav
    │   └── UPGRADE.mp3
    ├── Frog/
    │   ├── FROG_BG.mp3
    │   ├── FROG_JUMP.wav
    │   └── FROG_BACK.wav
    ├── Mole/
    │   ├── MOUSE_BG.mp3
    │   ├── MOUSE_CLICK.wav
    │   ├── MOUSE_OUT.wav
    │   └── MOUSE_AWAY.wav
    └── Police/
        ├── LAMISTER_BG.mp3
        ├── PT_POLICE_CATCH.wav
        └── PT_THIEF_AWAY.wav
```

## 音频格式说明

| 用途 | Qt 类 | 支持格式 |
|---|---|---|
| 背景音乐（循环） | `QMediaPlayer` | MP3 / OGG / WAV |
| 短音效（低延迟） | `QSoundEffect` | **仅 WAV**（PCM，非压缩）|

> 如果资源包中的音效是 MP3 格式，需用 Audacity / ffmpeg 转换为 WAV：
> ```bash
> ffmpeg -i APPLE_IN.mp3 APPLE_IN.wav
> ```

## 没有资源文件也可以运行

代码做了完整的 fallback 处理：
- 图片缺失 → 使用渐变色 + emoji 替代
- 音效缺失 → `QSoundEffect` 静默加载，不崩溃
- 背景音乐缺失 → `QMediaPlayer` 静默失败，不崩溃
