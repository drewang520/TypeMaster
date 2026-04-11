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
 * 负责游戏计时器、水果生成、水果移动、按键匹配
 * 以及关卡进度。直接修改 GameData；视图通过观察者通知
 * 自动更新自身。
 *
 * 连接示例（AppleWindow）：
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

private:
    void spawnFruit();
    void updateFruits();
    void updateLevel();

    GameData* m_model{nullptr};
    GameView* m_view{nullptr};
    QTimer*   m_gameTimer{nullptr};

    int m_tickCount     {0};
    int m_spawnInterval {60};
    int m_nextSpawn     {0};
};

#endif // GAME_CONTROLLER_H