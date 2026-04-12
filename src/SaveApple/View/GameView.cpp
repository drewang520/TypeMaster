#include "GameView.h"
#include "languagemanager.h"
#include <QPainter>
#include <QPainterPath>
#include <QKeyEvent>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QFont>
#include "SaveApple/Model/GameData.h"
#include "SaveApple/Model/Fruit.h"
#include "SaveApple/Config/GameConfig.h"

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

// ── Paint dispatch ──
void GameView::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    // 错误修复 2：HUD 标签在 AppleWindow 中是 Qt 控件——请勿在此处绘制它们。
    drawBackground(p);
    drawBasket(p);
    drawApples(p);

    if (m_gameData->isPaused() || m_gameData->isGameOver())
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

    // Ground strip
    const GameConfig& cfg = GameConfig::getInstance();
    p.setBrush(QColor(0x228B22));
    p.setPen(Qt::NoPen);
    p.drawRect(0, height() - cfg.getBasketOffsetY() * 2,
               width(), cfg.getBasketOffsetY() * 2);
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
    // Dim the whole view
    p.setBrush(QColor(0, 0, 0, 160));
    p.setPen(Qt::NoPen);
    p.drawRect(rect());

    const QPoint centre(width() / 2, height() / 2);
    QRect panel(centre.x() - 180, centre.y() - 130, 360, 260);

    p.setBrush(QColor(0x1A1A2E));
    p.setPen(QPen(QColor(0xFF6B6B), 2));
    p.drawRoundedRect(panel, 16, 16);

    // 国际化修复：GameView 是 MVC 重构中引入的一个新类。
    // 其 tr() 上下文为 “GameView”，而现有的
    // zh_CN.ts 文件中没有该条目。请使用 QCoreApplication::translate() 并保留原始的
    // “AppleWindow” 上下文，这样即可复用现有翻译，而无需重新运行 lupdate。
    auto t = [](const char* src) -> QString {
        return QCoreApplication::translate("AppleWindow", src);
    };

    p.setPen(Qt::white);
    if (m_gameData->isPaused()) {
        p.setFont(QFont(QStringLiteral("Arial"), 28, QFont::Bold));
        p.drawText(panel, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap,
                   QStringLiteral("\n") + t("Paused"));
        p.setFont(QFont(QStringLiteral("Arial"), 13));
        p.drawText(panel.adjusted(0, 90, 0, 0),
                   Qt::AlignHCenter | Qt::AlignTop,
                   t("Press Resume to continue"));
    } else {
        p.setFont(QFont(QStringLiteral("Arial"), 28, QFont::Bold));
        p.setPen(QColor(0xFF6B6B));
        p.drawText(panel, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap,
                   QStringLiteral("\n") + t("Game Over"));

        p.setPen(Qt::white);
        p.setFont(QFont(QStringLiteral("Arial"), 14));
        const QString stats =
            t("Score: %1    Level: %2\nCaught: %3    Missed: %4")
                .arg(m_gameData->getScore())
                .arg(m_gameData->getLevel())
                .arg(m_gameData->getCaughtCount())
                .arg(m_gameData->getMissedCount());

        // 布局调整：将统计信息限制在面板的上部，
        // 底部预留 80 像素的空白区域，用于放置 AppleWindow 定位在覆盖层上的“再次播放”QPushButton。
        p.drawText(panel.adjusted(10, 95, -10, -90),
                   Qt::AlignCenter | Qt::TextWordWrap, stats);
    }
}
