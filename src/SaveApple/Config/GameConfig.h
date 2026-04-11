#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

/**
 * GameConfig — 包含所有游戏常量的单例（配置层）。
 *
 * 将此前分散在 AppleWindow 各处的魔法数集中管理。
 * 构造完成后即为只读；无需观察者。
 *
 * 使用方法：
 *   int lives = GameConfig::getInstance().getInitialLives();
 */
class GameConfig
{
public:
    /** Return the single instance. */
    static GameConfig& getInstance()
    {
        static GameConfig instance;
        return instance;
    }

    // Disallow copy / assign
    GameConfig(const GameConfig&)            = delete;
    GameConfig& operator=(const GameConfig&) = delete;

    // ── Gameplay ──
    int   getInitialLives()           const { return 5;    }
    int   getMaxLives()               const { return 5;    }
    int   getMaxFruitsOnScreen()      const { return 8;    }
    int   getInitialSpawnInterval()   const { return 60;   } // ticks
    int   getMinSpawnInterval()       const { return 20;   }
    int   getSpawnDecrement()         const { return 5;    }
    int   getCatchesPerLevel()        const { return 10;   }
    int   getMaxBasketDisplay()       const { return 12;   }

    // ── Physics ──
    float getBaseSpeed()              const { return 1.5f; }
    float getSpeedPerLevel()          const { return 0.4f; }
    float getSpeedRandRange()         const { return 0.8f; }

    // ── Rendering ──
    int   getAppleRadius()            const { return 32;   }
    int   getBasketOffsetY()          const { return 20;   } // px from bottom
    int   getHudHeight()              const { return 50;   } // px

    // ── Timing ──
    int   getTickMs()                 const { return 30;   }
    int   getCaughtAnimFrames()       const { return 20;   }
    int   getBrokenAnimFrames()       const { return 25;   }

    // ── Scoring ──
    int   getBaseScorePerCatch()      const { return 10;   }

private:
    GameConfig() = default;
};

#endif // GAME_CONFIG_H