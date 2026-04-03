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
