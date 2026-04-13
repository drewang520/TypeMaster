#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <algorithm>
/**
 * GameConfig — 包含所有游戏常量的单例（配置层）。
 *
 * 不可变常量（物理参数、计时、渲染）采用内联获取器。
 * 可变设置（速度等级、苹果上限、关卡目标、音效）
 * 通过 SettingsDialog 进行修改，并在进程运行期间保持有效。
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

    // ── 不可变常量
    int   getInitialLives()           const { return 5;    }
    int   getMaxLives()               const { return 5;    }
    int   getAppleRadius()            const { return 32;   }
    int   getBasketOffsetY()          const { return 20;   } // px from bottom
    int   getHudHeight()              const { return 50;   } // px
    int   getTickMs()                 const { return 30;   }
    int   getCaughtAnimFrames()       const { return 20;   }
    int   getBrokenAnimFrames()       const { return 25;   }
    int   getMaxBasketDisplay()       const { return 12;   }
    int   getBaseScorePerCatch()      const { return 10;   }
    float getSpeedRandRange()         const { return 0.4f; }

    // 当苹果达到 GameView 高度的这一比例时，即被视为“漏接”（要求：70%）。
    float getGroundRatio()            const { return 0.70f; }

    // ── 可变的运行时设置（通过 SettingsDialog 更改） ──
    /** 速度等级 1-10：基础下落速度倍数。 */
    int  getSpeedLevel()  const { return m_speedLevel; }
    void setSpeedLevel(int v)   { m_speedLevel  = std::clamp(v, 1, 10); }

    /** 屏幕上同时显示的苹果数量上限（1-5）。 */
    int  getMaxFruitsOnScreen() const { return m_maxFruits; }
    void setMaxFruits(int v)          { m_maxFruits  = std::clamp(v, 1, 5);  }
 
    /** 每个关卡触发“关卡通关”前所需的捕获数量（5-30）。 */
    int  getLevelTarget()  const { return m_levelTarget; }
    void setLevelTarget(int v)   { m_levelTarget = std::clamp(v, 5, 30); }
 
    /** 全局音效/音乐切换。 */
    bool isSoundEnabled()  const { return m_soundEnabled; }
    void setSoundEnabled(bool v) { m_soundEnabled = v; }

    // ── 派生值（由可变设置计算得出） ──
    /** 给定游戏关卡下新水果的初始下落速度。 */
    float computeBaseSpeed(int gameLevel) const
    {
        // speedLevel 1→0.8 px/tick, speedLevel 10→3.5 px/tick
        const float base = 0.5f + m_speedLevel * 0.3f;
        return base + gameLevel * 0.25f;
    }
 
    /** 连续生成之间的间隔（随着 speedLevel 的增加而减少）。 */
    int getSpawnInterval() const
    {
        return std::max(15, 75 - m_speedLevel * 5);
    }
 
    // 关卡通关自动前进延迟（每30毫秒一个时间步长 ≈ 2 秒）
    int getLevelCompleteDelay() const { return 66; }


private:
    GameConfig() = default;

    int  m_speedLevel  {5};
    int  m_maxFruits   {5};
    int  m_levelTarget {10};
    bool m_soundEnabled{true};
};

#endif // GAME_CONFIG_H