#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <vector>
#include <QVector>
#include <QChar>
#include "Fruit.h"
#include "../../IObserver.h"

/** 苹果游戏会话的当前生命周期状态。 */
enum class GameState { Ready, Running, Paused, GameOver };

/**
 * GameData — 游戏模型层（MVC 中的“M”）。
 *
 * 管理所有可变的游戏状态：
 *   - 得分、生命值、关卡、统计数据
 *   - 当前活跃的 Fruit 实体列表
 *   - 篮筐显示列表
 *
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

    // ── 统计数据 ──
    int  getCaughtCount() const { return m_caughtCount; }
    int  getMissedCount() const { return m_missedCount; }
    void incrementCaught();
    void incrementMissed();

    // ── 游戏状态 ──
    GameState getState()  const { return m_state; }
    void      setState(GameState state);
    bool      isGameOver() const { return m_state == GameState::GameOver; }
    bool      isPaused()   const { return m_state == GameState::Paused;   }
    bool      isRunning()  const { return m_state == GameState::Running;  }

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

    GameState       m_state       {GameState::Ready};
    QVector<Fruit>  m_fruits;
    QVector<QChar>  m_basketApples;

    std::vector<IObserver*> m_observers;
};

#endif // GAME_DATA_H