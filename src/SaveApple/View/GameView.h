#ifndef GAME_VIEW_H
#define GAME_VIEW_H

#include <QWidget>
#include <QPixmap>
#include "../../IObserver.h"

class GameData;

/**
 * GameView — 游戏渲染组件（MVC 中的“V”）。
 *
 * 实现 IObserver 接口：当 GameData 调用 notifyObservers() 时，
 * onUpdate() 会被触发并安排重绘操作 — HUD 和
 * 游戏场景始终与模型保持同步。
 *
 * 视图（View）对模型而言是只读的：它绝不会
 * 直接修改 GameData。按键事件会通过
 * keyPressed() 信号转发给控制器（Controller）。
 */

class GameView : public QWidget, public IObserver
{
    Q_OBJECT

public:
    explicit GameView(GameData* data, QWidget* parent = nullptr);
    ~GameView() override = default;

    // IObserver — called by GameData::notifyObservers()
    void onUpdate() override;

signals:
    /** 当游戏处于焦点状态时，用户按下按键会触发此事件。 */
    void keyPressed(QChar ch);

protected:
    void paintEvent(QPaintEvent* event)  override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    // ── 渲染阶段 ──
    void drawBackground(QPainter& p);
    void drawApples(QPainter& p);
    void drawBasket(QPainter& p);
    void drawHud(QPainter& p);
    void drawOverlay(QPainter& p);

    GameData* m_gameData{nullptr};

    // ── Sprite resources (graceful null-pixmap fallback) ──
    QPixmap m_bgPixmap;
    QPixmap m_appleNormal;
    QPixmap m_appleBad;
    QPixmap m_appleBasket;
    QPixmap m_appleSmall;
};

#endif // GAME_VIEW_H