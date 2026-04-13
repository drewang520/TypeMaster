#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QChar>

class GameData;
class GameView;

/**
 * GameController — 游戏逻辑控制器（MVC 中的“C”）。
 *
 * 职责：
 *   - 驱动游戏计时器（onTick）
 *   - 生成水果（需满足不重复字母的约束条件）
 *   - 移动水果；检测水果与地平线偏离70%的情况
 *   - 将键盘输入与水果进行匹配
 *   - 检测关卡完成（每关目标）和游戏结束
 *   - 在LevelComplete延迟后自动进入下一关
 *
 *   connect(m_gameView, &GameView::keyPressed,
 *           m_controller, &GameController::handleKeyPress);
 */
class GameController : public QObject
{
    Q_OBJECT

public:
    explicit GameController(GameData* model, GameView* view,
                            QObject* parent = nullptr);
    ~GameController() override = default;

public slots:
    void startGame();
    void pauseGame();
    void resumeGame();
    void restartGame();

    /** Called when a key is typed; tries to match the letter to a fruit. */
    void handleKeyPress(QChar ch);

signals:
    /** Emitted when a fruit is successfully caught (used to trigger SFX). */
    void fruitCaught();

    /** Emitted whenever the game state changes (used to update HUD buttons). */
    void gameStateChanged();

private slots:
    void onTick();
    /** 当 LevelComplete 显示超时后，由 m_levelTimer 调用。 */
    void onLevelCompleteEnd();

private:
    void spawnFruit();
    void updateFruits();
    void checkLevelComplete();

    GameData* m_model{nullptr};
    GameView* m_view{nullptr};
    QTimer*   m_gameTimer{nullptr};
    QTimer* m_levelTimer {nullptr};   // 单次触发：关卡通关后自动跳过

    int m_tickCount     {0};
    // int m_spawnInterval {60};
    int m_nextSpawn     {0};
};

#endif // GAME_CONTROLLER_H