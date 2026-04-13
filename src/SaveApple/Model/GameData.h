#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <vector>
#include <QVector>
#include <QSet>
#include <QChar>
#include "Fruit.h"
#include "IObserver.h"

 /** GameState — 游戏会话的完整生命周期。
 *
 * Ready        → 游戏尚未开始（显示“开始”按钮）
 * Running      → 正常游戏
 * Paused       → 计时器停止（显示“继续”按钮）
 * LevelComplete→ 玩家达到该关卡的得分目标；
 *                显示覆盖层约 2 秒，随后自动进入下一关
 * GameOver     → 生命值耗尽
 */ 
enum class GameState { Ready, Running, Paused, LevelComplete, GameOver };

/**
 * GameData — 游戏模型层（MVC 中的“M”）。
 *
 * 管理所有可变的游戏状态：
 *   - 得分、生命值、关卡、统计数据
 *   - 当前活跃的 Fruit 实体列表
 *   - 篮筐显示列表
 *  拥有所有可变的游戏状态，并在每次状态变更后通知观察者。
 * 状态发生任何变化后，会通知已注册的 IObserver 实例，
 * 因此视图会自动刷新，无需手动调用 updateHud()。
 */

class GameData
{
public:
    GameData();

    // ── 分数 ──
    int  getScore()  const { return m_score; }
    void addScore(int value);

    // ── 生命值 ──
    int  getLives()    const { return m_lives;    }
    int  getMaxLives() const { return m_maxLives; }
    void loseLife();
    void addLife();

    // ── 关卡 ──
    int  getLevel() const { return m_level; }
    void setLevel(int level);

    // ── 每关捕获计数器 ──
    /** 当前关卡中累计捕获的数量（升级后重置）。 */
    int  getLevelCaught()   const { return m_levelCaught; }
    void incrementLevelCaught();
    void resetLevelCaught();

    // ── 统计数据 ──
    int  getCaughtCount() const { return m_caughtCount; }
    int  getMissedCount() const { return m_missedCount; }
    void incrementCaught();
    void incrementMissed();

    // ── 活跃字母追踪（用于防止重复约束） ───────────
    /** 返回当前显示在屏幕上的字母集合（下落的水果）。 */
    const QSet<QChar>& getActiveLetters() const { return m_activeLetters; }
    void addActiveLetter(QChar ch)    { m_activeLetters.insert(ch); }
    void removeActiveLetter(QChar ch) { m_activeLetters.remove(ch); }

    // ── 游戏状态 ──
    GameState getState()  const { return m_state; }
    void setState(GameState state);
    bool isGameOver() const { return m_state == GameState::GameOver; }
    bool isPaused()   const { return m_state == GameState::Paused;   }
    bool isRunning()  const { return m_state == GameState::Running;  }
    bool isLevelComplete() const { return m_state == GameState::LevelComplete; }
    bool isReady()         const { return m_state == GameState::Ready;         }

    // ── 水果集合（在此处拥有，由控制器修改） ──
    QVector<Fruit>&       getFruits()       { return m_fruits; }
    const QVector<Fruit>& getFruits() const { return m_fruits; }

    // ── 篮子显示列表 ──
    QVector<QChar>&       getBasketApples()       { return m_basketApples; }
    const QVector<QChar>& getBasketApples() const { return m_basketApples; }

    // ── 完全重置 ──
    /** 将所有字段重置为初始值，并通知观察者。 */
    void reset();

    // ── 观察者模式 ──
    void attach(IObserver* observer);
    void detach(IObserver* observer);
    void notifyObservers();

private:
    int m_score      {0};
    int m_lives      {5};
    int m_maxLives   {5};
    int m_level      {1};
    int m_caughtCount{0};
    int m_missedCount{0};
    int m_levelCaught {0};   // 仅捕获当前层级的内容

    GameState       m_state       {GameState::Ready};
    QVector<Fruit>  m_fruits;
    QVector<QChar>  m_basketApples;
    QSet<QChar>    m_activeLetters;

    std::vector<IObserver*> m_observers;
};

#endif // GAME_DATA_H