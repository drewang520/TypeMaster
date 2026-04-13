#include "GameView.h"
#include "languagemanager.h"
#include "SaveApple/Model/GameData.h"
#include "SaveApple/Model/Fruit.h"
#include "SaveApple/Config/GameConfig.h"
#include <QPainter>
#include <QPainterPath>
#include <QKeyEvent>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QFont>
#include <cmath>

// ── Paw-circle positions (pixel coordinates in the original image) ────────────
//
// Layout in APPLE_BACKGROUND.png, origin = top-left corner:
//
//        ● (183, 499)   ← top circle
//   ● (137, 533)           ● (221, 550)   ← left / right circles
//        ● (167, 571)   ← bottom circle
//
// These were measured directly in the source image.
// To recalibrate: open APPLE_BACKGROUND.png in any image editor,
// hover over each circle centre, read (px, py), update the table.
//
// The values are stored as raw pixels; getPawCenters() divides by the
// loaded pixmap size at runtime, so this works for any image resolution.
//
const QPointF GameView::kPawFractions[GameView::kPawCount] = {
    { 184.0, 503.0 },   // top circle
    { 138.0, 533.0 },   // left circle
    { 170.0, 570.0 },   // bottom circle
    { 222.0, 553.0 },   // right circle
};

const double GameView::kPawRadii[GameView::kPawCount] = {
    23.0,   // top    (开始 — slightly larger)
    22.0,   // left   (暂停)
    22.0,   // bottom (设置)
    28.0,   // right  (结束 — visibly larger in the image)
};


// ── Constructor ──
GameView::GameView(GameData* data, QWidget* parent)
    : QWidget(parent)
    , m_gameData(data)
{
    setFocusPolicy(Qt::StrongFocus);
    m_bgPixmap    = QPixmap(QStringLiteral(":/images/Apple/APPLE_BACKGROUND.png"));
    m_appleNormal = QPixmap(QStringLiteral(":/images/Apple/APPLE_NORMAL.png"));
    m_appleBad    = QPixmap(QStringLiteral(":/images/Apple/APPLE_BAD.png"));
    m_appleBasket = QPixmap(QStringLiteral(":/images/Apple/APPLE_BASKET.png"));
    m_appleSmall  = QPixmap(QStringLiteral(":/images/Apple/APPLE_SMALL.png"));
}

// ── IObserver ──
void GameView::onUpdate()
{
    update(); // schedule repaint
}

// ── Key forwarding ──
void GameView::keyPressEvent(QKeyEvent* event)
{
    const QString text = event->text().toUpper();
    if (!text.isEmpty())
        emit keyPressed(text[0]);
    QWidget::keyPressEvent(event);
}

// ── Background transform (mirrors Qt's KeepAspectRatioByExpanding logic) ──
// void GameView::computeBgTransform(double& scale, QPointF& offset) const
// {
//     if (m_bgPixmap.isNull() || m_bgPixmap.width() == 0 || m_bgPixmap.height() == 0) {
//         scale  = 1.0;
//         offset = {0.0, 0.0};
//         return;
//     }
 
//     const double imgW = m_bgPixmap.width();
//     const double imgH = m_bgPixmap.height();
//     const double vW   = width();
//     const double vH   = height();
 
//     // KeepAspectRatioByExpanding: largest scale so the image covers the widget
//     scale = std::max(vW / imgW, vH / imgH);
 
//     // The scaled image is centred; offset can be negative (crop)
//     offset = QPointF((vW - imgW * scale) / 2.0,
//                      (vH - imgH * scale) / 2.0);
// }

// ── Public: paw-circle positions ──
QVector<QPoint> GameView::getPawCenters() const
{
    if (m_bgPixmap.isNull() || m_bgPixmap.width() == 0 || m_bgPixmap.height() == 0)
        return {};
 
    const double scaleX = static_cast<double>(width())  / m_bgPixmap.width();
    const double scaleY = static_cast<double>(height()) / m_bgPixmap.height();
 
    QVector<QPoint> centres;
    centres.reserve(kPawCount);
    for (int i = 0; i < kPawCount; ++i) {
        centres.append(QPoint(
            static_cast<int>(std::round(kPawFractions[i].x() * scaleX)),
            static_cast<int>(std::round(kPawFractions[i].y() * scaleY))
        ));
    }
    return centres;
}
 
QVector<int> GameView::getPawRadii() const
{
    if (m_bgPixmap.isNull() || m_bgPixmap.width() == 0 || m_bgPixmap.height() == 0)
        return { 24, 24, 24, 24 };
 
    // Use the smaller axis scale so the button fits inside the circle
    // regardless of whether the window is wider or taller than the image.
    const double scaleX = static_cast<double>(width())  / m_bgPixmap.width();
    const double scaleY = static_cast<double>(height()) / m_bgPixmap.height();
    const double scale  = std::min(scaleX, scaleY);
 
    QVector<int> radii;
    radii.reserve(kPawCount);
    for (int i = 0; i < kPawCount; ++i)
        radii.append(std::max(12, static_cast<int>(std::round(kPawRadii[i] * scale))));
    return radii;
}


// ── Paint dispatch ──
void GameView::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    // 错误修复 2：HUD 标签在 AppleWindow 中是 Qt 控件——请勿在此处绘制它们。
    drawBackground(p);
    drawGroundLine(p);
    drawBasket(p);
    drawApples(p);

    const GameState st = m_gameData->getState();
    if (st == GameState::Paused       ||
        st == GameState::LevelComplete ||
        st == GameState::GameOver)
        drawOverlay(p);
}

// ── Background ──
void GameView::drawBackground(QPainter& p)
{
    if (!m_bgPixmap.isNull()) {
        p.drawPixmap(rect(),
                     m_bgPixmap.scaled(size(),
                                       Qt::KeepAspectRatioByExpanding,
                                       Qt::SmoothTransformation));
        return;
    }

    // Gradient sky fallback
    QLinearGradient sky(0, 0, 0, height());
    sky.setColorAt(0.0, QColor(0x87CEEB));
    sky.setColorAt(0.6, QColor(0xB0E0E6));
    sky.setColorAt(1.0, QColor(0x228B22));
    p.fillRect(rect(), sky);

}

// ── Ground line (70% mark) ──
void GameView::drawGroundLine(QPainter& p)
{
    const int groundY = static_cast<int>(
        height() * GameConfig::getInstance().getGroundRatio());
 
    // Subtle dashed line so players can see the danger zone
    QPen pen(QColor(255, 80, 80, 100), 1, Qt::DashLine);
    p.setPen(pen);
    p.drawLine(0, groundY, width(), groundY);
}

// ── Basket ──
void GameView::drawBasket(QPainter& p)
{
    const int bw = 120, bh = 80;
    const int bx = width()  - bw - 20;
    const int by = height() - bh - 10;

    if (!m_appleBasket.isNull()) {
        p.drawPixmap(bx, by, bw, bh, m_appleBasket);
    } else {
        p.setBrush(QColor(0xA0522D));
        p.setPen(QPen(QColor(0x5C3317), 2));
        QPainterPath basket;
        basket.moveTo(bx + 10, by);
        basket.lineTo(bx + bw - 10, by);
        basket.lineTo(bx + bw, by + bh);
        basket.lineTo(bx,      by + bh);
        basket.closeSubpath();
        p.drawPath(basket);
    }

    // Small apples already in basket
    const QVector<QChar>& ba = m_gameData->getBasketApples();
    if (!m_appleSmall.isNull()) {
        int row = 0, col = 0;
        for (int i = 0; i < ba.size() && i < 12; ++i) {
            p.drawPixmap(bx + 10 + col * 18, by + 5 + row * 18,
                         16, 16, m_appleSmall);
            if (++col >= 6) { col = 0; ++row; }
        }
    } else {
        p.setBrush(QColor(0xFF4444));
        p.setPen(Qt::NoPen);
        int row = 0, col = 0;
        for (int i = 0; i < ba.size() && i < 12; ++i) {
            p.drawEllipse(bx + 14 + col * 18, by + 10 + row * 18, 12, 12);
            if (++col >= 6) { col = 0; ++row; }
        }
    }
}

// ── Apples ──

void GameView::drawApples(QPainter& p)
{
    const int r = GameConfig::getInstance().getAppleRadius();

    for (const Fruit& fruit : m_gameData->getFruits()) {
        const int ax = static_cast<int>(fruit.getX()) - r;
        const int ay = static_cast<int>(fruit.getY()) - r;

        if (fruit.getState() == Fruit::State::Caught) {
            // Catch flash
            const qreal alpha = static_cast<qreal>(fruit.getAnimFrame()) / 20.0;
            p.setOpacity(alpha);
            p.setBrush(QColor(255, 255, 100));
            p.setPen(Qt::NoPen);
            p.drawEllipse(ax - 4, ay - 4, r * 2 + 8, r * 2 + 8);
            p.setOpacity(1.0);

        } else if (fruit.getState() == Fruit::State::Broken) {
            // Broken fade-out
            const qreal alpha = static_cast<qreal>(fruit.getAnimFrame()) / 25.0;
            p.setOpacity(alpha);
            if (!m_appleBad.isNull()) {
                p.drawPixmap(ax, ay, r * 2, r * 2, m_appleBad);
            } else {
                p.setBrush(QColor(0x8B2500));
                p.setPen(Qt::NoPen);
                p.drawEllipse(ax, ay, r * 2, r * 2);
            }
            p.setOpacity(1.0);

        } else {
            // Normal falling apple
            if (!m_appleNormal.isNull()) {
                p.drawPixmap(ax, ay, r * 2, r * 2, m_appleNormal);
            } else {
                const float cx = fruit.getX();
                const float cy = fruit.getY();
                QRadialGradient radial(cx - r / 3, cy - r / 3, r);
                radial.setColorAt(0.0, QColor(0xFF6666));
                radial.setColorAt(0.7, QColor(0xCC1111));
                radial.setColorAt(1.0, QColor(0x880000));
                p.setBrush(radial);
                p.setPen(QPen(QColor(0x660000), 1));
                p.drawEllipse(ax, ay, r * 2, r * 2);
                // Shine
                p.setBrush(QColor(255, 255, 255, 80));
                p.setPen(Qt::NoPen);
                p.drawEllipse(ax + r / 2 - 4, ay + r / 2 - 6, 10, 7);
                // Stem
                p.setPen(QPen(QColor(0x5C3317), 2));
                p.drawLine(static_cast<int>(cx), ay,
                           static_cast<int>(cx) + 4, ay - 7);
            }

            // Dark circle behind letter
            const int cx = static_cast<int>(fruit.getX());
            const int cy = static_cast<int>(fruit.getY());
            p.setBrush(QColor(0, 0, 0, 100));
            p.setPen(Qt::NoPen);
            p.drawEllipse(cx - 11, cy - 11, 22, 22);

            // Letter
            p.setPen(Qt::white);
            p.setFont(QFont(QStringLiteral("Arial"), 16, QFont::Bold));
            p.drawText(QRect(ax, ay, r * 2, r * 2),
                       Qt::AlignCenter, fruit.getLetter());
        }
    }
}

// ── Overlay (pause / game-over) ──
void GameView::drawOverlay(QPainter& p)
{
    // i18n: use "AppleWindow" context so existing zh_CN translations apply
    // // 国际化修复：GameView 是 MVC 重构中引入的一个新类。
    // // 其 tr() 上下文为 “GameView”，而现有的
    // // zh_CN.ts 文件中没有该条目。请使用 QCoreApplication::translate() 并保留原始的
    // // “AppleWindow” 上下文，这样即可复用现有翻译，而无需重新运行 lupdate。
    auto t = [](const char* src) -> QString {
        return QCoreApplication::translate("AppleWindow", src);
    };

    // Dim the whole view
    p.setBrush(QColor(0, 0, 0, 160));
    p.setPen(Qt::NoPen);
    p.drawRect(rect());

    const QPoint centre(width() / 2, height() / 2);
    QRect panel(centre.x() - 180, centre.y() - 130, 360, 260);

    p.setBrush(QColor(0x1A1A2E));
    p.setPen(QPen(QColor(0xFF6B6B), 2));
    p.drawRoundedRect(panel, 16, 16);

    const GameState st = m_gameData->getState();

    if (st == GameState::Paused) {
        // ── Paused ──
        p.setPen(Qt::white);
        p.setFont(QFont(QStringLiteral("Arial"), 28, QFont::Bold));
        p.drawText(panel, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap,
                   QStringLiteral("\n") + t("Paused"));
        p.setFont(QFont(QStringLiteral("Arial"), 13));
        p.drawText(panel.adjusted(0, 90, 0, 0),
                   Qt::AlignHCenter | Qt::AlignTop,
                   t("Press Resume to continue"));
 
    } else if (st == GameState::LevelComplete) {
        // ── Level Complete ──
        p.setPen(QColor(0x4CAF50));
        p.setFont(QFont(QStringLiteral("Arial"), 26, QFont::Bold));
        p.drawText(panel, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap,
                   QStringLiteral("\n") + t("Level Complete!"));
 
        p.setPen(Qt::white);
        p.setFont(QFont(QStringLiteral("Arial"), 15));
        p.drawText(panel.adjusted(0, 95, 0, 0),
                   Qt::AlignHCenter | Qt::AlignTop,
                   t("Level %1  →  Level %2")
                       .arg(m_gameData->getLevel())
                       .arg(m_gameData->getLevel() + 1));
 
        p.setFont(QFont(QStringLiteral("Arial"), 12));
        p.setPen(QColor(180, 180, 180));
        p.drawText(panel.adjusted(0, 135, 0, 0),
                   Qt::AlignHCenter | Qt::AlignTop,
                   t("Get ready..."));
 
    } else {
        // ── Game Over ──
        p.setPen(QColor(0xFF6B6B));
        p.setFont(QFont(QStringLiteral("Arial"), 28, QFont::Bold));
        p.drawText(panel, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap,
                   QStringLiteral("\n") + t("Game Over"));
 
        p.setPen(Qt::white);
        p.setFont(QFont(QStringLiteral("Arial"), 14));
        const QString stats =
            QString(t("Score: %1    Level: %2\nCaught: %3    Missed: %4"))
                .arg(m_gameData->getScore())
                .arg(m_gameData->getLevel())
                .arg(m_gameData->getCaughtCount())
                .arg(m_gameData->getMissedCount());
        // Leave bottom 80 px clear for the "Play Again" QPushButton
        p.drawText(panel.adjusted(10, 95, -10, -90),
                   Qt::AlignCenter | Qt::TextWordWrap, stats);
    }
}
