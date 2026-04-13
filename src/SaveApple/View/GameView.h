#ifndef GAME_VIEW_H
#define GAME_VIEW_H

#include "IObserver.h"
#include <QWidget>
#include <QPixmap>
#include <QVector>
#include <QPoint>
#include <QPointF>

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

    /**
     * Returns the centres of the 4 paw-print circles in this widget's
     * coordinate space, correctly accounting for the background image
     * scaling (KeepAspectRatioByExpanding).
     *
     * Returns an empty vector if the background pixmap is not loaded
     * (caller should fall back to a default layout).
     *
     * The 4 positions are defined as fractions of the *original* image
     * size in kPawFractions[]. Adjust those constants if the background
     * image is replaced.
     */
    QVector<QPoint> getPawCenters() const;
 
    /**
     * Radius (in widget pixels) of one paw circle at current scale.
     * Use this to size the overlay buttons so they fit inside the circles.
     */
    QVector<int> getPawRadii() const;

signals:
    /** 当游戏处于焦点状态时，用户按下按键会触发此事件。 */
    void keyPressed(QChar ch);

protected:
    void paintEvent(QPaintEvent* event)  override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    // ── 渲染阶段 ──
    // 注意：HUD 标签（分数/生命值/关卡）是 Qt 控件标签
    // 由 AppleWindow 管理——它们不会在此处绘制。
    void drawBackground(QPainter& p);
    void drawGroundLine(QPainter& p);  // 70% danger line
    void drawApples(QPainter& p);
    void drawBasket(QPainter& p);
    void drawOverlay(QPainter& p);  // Paused / LevelComplete / GameOver

    /**
     * Computes the scale factor and top-left offset that Qt uses when
     * drawing m_bgPixmap with KeepAspectRatioByExpanding into this widget.
     *
     * scale  — uniform scale applied to the source image
     * offset — top-left corner of the scaled image in widget coords
     *          (negative components mean that side is cropped)
     */
    // void computeBgTransform(double& scale, QPointF& offset) const;

    GameData* m_gameData{nullptr};

    // ── Sprite resources (graceful null-pixmap fallback) ──
    QPixmap m_bgPixmap;
    QPixmap m_appleNormal;
    QPixmap m_appleBad;
    QPixmap m_appleBasket;
    QPixmap m_appleSmall;

    // ── Paw-circle positions in the original background image ─────────
    // Each QPointF is (x_fraction, y_fraction) of the original image size.
    // Measured from the top-left corner of APPLE_BACKGROUND.png.
    //
    // How to re-calibrate if the image changes:
    //   1. Open APPLE_BACKGROUND.png in any image editor
    //   2. Find the pixel centre of each circle
    //   3. Divide by image width/height to get fractions
    //   4. Update the values below
    //
    // Current values estimated from the visible paw-print at bottom-left.
    static constexpr int kPawCount = 4;
    static const QPointF kPawFractions[kPawCount];
 
    // Radius of each circle in the original image (pixels), used to
    // scale the button size proportionally.
    // static constexpr double kPawRadiusPx = 22.0;
    static const double kPawRadii[kPawCount];

};

#endif // GAME_VIEW_H