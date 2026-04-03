#include "applewindow.h"

#include <QPainter>
#include <QPainterPath>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QtMath>

// ── Constructor ───────────────────────────────────────────────────────────────

AppleWindow::AppleWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(tr("Save the Apple"));
    setMinimumSize(700, 550);
    resize(800, 620);
    setFocusPolicy(Qt::StrongFocus);

    // Load resources (graceful fallback if not found)
    m_bgPixmap    = QPixmap(QStringLiteral(":/images/Apple/APPLE_BACKGROUND.png"));
    m_appleNormal = QPixmap(QStringLiteral(":/images/Apple/APPLE_NORMAL.png"));
    m_appleBad    = QPixmap(QStringLiteral(":/images/Apple/APPLE_BAD.png"));
    m_appleBasket = QPixmap(QStringLiteral(":/images/Apple/APPLE_BASKET.png"));
    m_appleSmall  = QPixmap(QStringLiteral(":/images/Apple/APPLE_SMALL.png"));

    setupUi();

    m_gameTimer = new QTimer(this);
    m_gameTimer->setInterval(kTickMs);
    connect(m_gameTimer, &QTimer::timeout, this, &AppleWindow::onTick);
}

// ── UI setup ──────────────────────────────────────────────────────────────────

void AppleWindow::setupUi()
{
    // HUD top bar
    QWidget *hud = new QWidget(this);
    hud->setObjectName(QStringLiteral("appleHud"));
    hud->setFixedHeight(50);
    hud->move(0, 0);

    QHBoxLayout *hudLayout = new QHBoxLayout(hud);
    hudLayout->setContentsMargins(12, 0, 12, 0);
    hudLayout->setSpacing(20);

    m_backBtn = new QPushButton(tr("◀ Back"), hud);
    m_backBtn->setObjectName(QStringLiteral("hudButton"));
    m_backBtn->setFixedWidth(90);
    connect(m_backBtn, &QPushButton::clicked, this, &QWidget::close);

    m_scoreLabel = new QLabel(tr("Score: 0"), hud);
    m_scoreLabel->setObjectName(QStringLiteral("hudLabel"));

    m_livesLabel = new QLabel(tr("Lives: ♥♥♥♥♥"), hud);
    m_livesLabel->setObjectName(QStringLiteral("hudLabel"));

    m_levelLabel = new QLabel(tr("Level: 1"), hud);
    m_levelLabel->setObjectName(QStringLiteral("hudLabel"));

    m_pauseBtn = new QPushButton(tr("Pause"), hud);
    m_pauseBtn->setObjectName(QStringLiteral("hudButton"));
    m_pauseBtn->setFixedWidth(80);
    connect(m_pauseBtn, &QPushButton::clicked, this, [this]() {
        if (m_state == GameState::Running) pauseGame();
        else if (m_state == GameState::Paused) resumeGame();
    });

    hudLayout->addWidget(m_backBtn);
    hudLayout->addStretch();
    hudLayout->addWidget(m_scoreLabel);
    hudLayout->addWidget(m_livesLabel);
    hudLayout->addWidget(m_levelLabel);
    hudLayout->addStretch();
    hudLayout->addWidget(m_pauseBtn);

    // Start button (centred, shown on Ready/GameOver)
    m_startBtn = new QPushButton(tr("Start Game"), this);
    m_startBtn->setObjectName(QStringLiteral("bigButton"));
    m_startBtn->setFixedSize(200, 55);
    connect(m_startBtn, &QPushButton::clicked, this, [this]() {
        if (m_state == GameState::GameOver) resetGame();
        startGame();
    });
}

// ── Resize handling ───────────────────────────────────────────────────────────

void AppleWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // HUD spans full width
    if (m_scoreLabel && m_scoreLabel->parentWidget())
        m_scoreLabel->parentWidget()->resize(width(), 50);
    // Centre start button in the lower half
    if (m_startBtn)
        m_startBtn->move((width() - m_startBtn->width()) / 2,
                         height() / 2 + 40);
}

// ── HUD text update ───────────────────────────────────────────────────────────

void AppleWindow::updateHud()
{
    m_scoreLabel->setText(tr("Score: %1").arg(m_score));

    QString hearts;
    for (int i = 0; i < kMaxLives; ++i)
        hearts += (i < m_lives) ? QStringLiteral("♥") : QStringLiteral("♡");
    m_livesLabel->setText(tr("Lives: %1").arg(hearts));

    m_levelLabel->setText(tr("Level: %1").arg(m_level));

    bool running = (m_state == GameState::Running || m_state == GameState::Paused);
    m_pauseBtn->setVisible(running);
    m_pauseBtn->setText(m_state == GameState::Paused ? tr("Resume") : tr("Pause"));
    m_startBtn->setVisible(!running);
    m_startBtn->setText(m_state == GameState::GameOver ? tr("Play Again") : tr("Start Game"));
}

// ── Game control ──────────────────────────────────────────────────────────────

void AppleWindow::startGame()
{
    m_state      = GameState::Running;
    m_tickCount  = 0;
    m_nextSpawn  = 0;
    m_gameTimer->start();
    updateHud();
    setFocus();
}

void AppleWindow::pauseGame()
{
    m_state = GameState::Paused;
    m_gameTimer->stop();
    updateHud();
    update();
}

void AppleWindow::resumeGame()
{
    m_state = GameState::Running;
    m_gameTimer->start();
    updateHud();
    setFocus();
}

void AppleWindow::endGame()
{
    m_state = GameState::GameOver;
    m_gameTimer->stop();
    updateHud();
    update();
}

void AppleWindow::resetGame()
{
    m_apples.clear();
    m_basketApples.clear();
    m_score       = 0;
    m_lives       = kMaxLives;
    m_level       = 1;
    m_caughtCount = 0;
    m_missedCount = 0;
    m_tickCount   = 0;
    m_nextSpawn   = 0;
    m_spawnInterval = 60;
    updateHud();
}

// ── Apple spawning ────────────────────────────────────────────────────────────

void AppleWindow::spawnApple()
{
    // Avoid spawning too many at once
    if (m_apples.size() >= 8) return;

    AppleItem item;
    // Random uppercase letter
    item.letter = QChar('A' + QRandomGenerator::global()->bounded(26));
    // Random x position (avoid edges)
    const int margin = kAppleRadius + 10;
    item.x = QRandomGenerator::global()->bounded(margin, width() - margin);
    item.y = 60.0; // just below HUD
    // Speed increases with level
    item.speed = 1.5 + m_level * 0.4
               + QRandomGenerator::global()->generateDouble() * 0.8;
    item.state = AppleItem::State::Falling;
    item.animFrame = 0;

    m_apples.append(item);
}

// ── Core game tick ────────────────────────────────────────────────────────────

void AppleWindow::onTick()
{
    ++m_tickCount;
    updateApples();

    // Spawn logic
    if (m_tickCount >= m_nextSpawn) {
        spawnApple();
        m_nextSpawn = m_tickCount + m_spawnInterval;
    }

    // Level up every 10 catches
    int newLevel = 1 + m_caughtCount / 10;
    if (newLevel != m_level) {
        m_level = newLevel;
        // Gradually decrease spawn interval (harder)
        m_spawnInterval = qMax(20, 60 - (m_level - 1) * 5);
    }

    updateHud();
    update(); // repaint
}

void AppleWindow::updateApples()
{
    const int groundY = height() - kBasketY - kAppleRadius;

    for (auto &apple : m_apples) {
        if (apple.state == AppleItem::State::Falling) {
            apple.y += apple.speed;

            // Apple reached ground — missed!
            if (apple.y >= groundY) {
                apple.state = AppleItem::State::Broken;
                apple.animFrame = 25; // show broken for 25 ticks
                --m_lives;
                ++m_missedCount;
                if (m_lives <= 0) {
                    m_lives = 0;
                    endGame();
                    return;
                }
            }
        } else {
            // Count down animation
            if (apple.animFrame > 0) --apple.animFrame;
        }
    }

    // Remove finished animations
    m_apples.erase(
        std::remove_if(m_apples.begin(), m_apples.end(),
            [](const AppleItem &a) {
                return a.state != AppleItem::State::Falling && a.animFrame == 0;
            }),
        m_apples.end()
    );
}

// ── Key input ─────────────────────────────────────────────────────────────────

void AppleWindow::keyPressEvent(QKeyEvent *event)
{
    if (m_state != GameState::Running) {
        QWidget::keyPressEvent(event);
        return;
    }

    const QString text = event->text().toUpper();
    if (text.isEmpty()) return;

    tryTypeLetter(text[0]);
}

bool AppleWindow::tryTypeLetter(QChar ch)
{
    // Find the lowest matching falling apple (most urgent)
    int bestIdx = -1;
    qreal bestY = -1.0;

    for (int i = 0; i < m_apples.size(); ++i) {
        const AppleItem &a = m_apples[i];
        if (a.state == AppleItem::State::Falling && a.letter == ch) {
            if (a.y > bestY) {
                bestY = a.y;
                bestIdx = i;
            }
        }
    }

    if (bestIdx < 0) return false;

    // Catch the apple
    AppleItem &caught = m_apples[bestIdx];
    caught.state     = AppleItem::State::Caught;
    caught.animFrame = 20; // show caught animation for 20 ticks

    m_score += 10 * m_level;
    ++m_caughtCount;
    if (m_basketApples.size() < 12)
        m_basketApples.append(ch);

    return true;
}

// ── Painting ──────────────────────────────────────────────────────────────────

void AppleWindow::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    drawBackground(p);
    drawBasket(p);
    drawApples(p);
    if (m_state == GameState::Paused || m_state == GameState::GameOver)
        drawOverlay(p);
}

void AppleWindow::drawBackground(QPainter &p)
{
    if (!m_bgPixmap.isNull()) {
        p.drawPixmap(rect(), m_bgPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding,
                                               Qt::SmoothTransformation));
    } else {
        // Gradient sky fallback
        QLinearGradient sky(0, 0, 0, height());
        sky.setColorAt(0.0, QColor(0x87CEEB));
        sky.setColorAt(0.6, QColor(0xB0E0E6));
        sky.setColorAt(1.0, QColor(0x228B22));
        p.fillRect(rect(), sky);

        // Ground strip
        p.setBrush(QColor(0x228B22));
        p.setPen(Qt::NoPen);
        p.drawRect(0, height() - kBasketY * 2, width(), kBasketY * 2);
    }
}

void AppleWindow::drawBasket(QPainter &p)
{
    const int bw = 120, bh = 80;
    const int bx = width() - bw - 20;
    const int by = height() - bh - 10;

    if (!m_appleBasket.isNull()) {
        p.drawPixmap(bx, by, bw, bh, m_appleBasket);
    } else {
        // Draw a simple basket shape
        p.setBrush(QColor(0xA0522D));
        p.setPen(QPen(QColor(0x5C3317), 2));
        QPainterPath basket;
        basket.moveTo(bx + 10, by);
        basket.lineTo(bx + bw - 10, by);
        basket.lineTo(bx + bw, by + bh);
        basket.lineTo(bx, by + bh);
        basket.closeSubpath();
        p.drawPath(basket);
    }

    // Small apples in basket
    if (!m_appleSmall.isNull()) {
        int row = 0, col = 0;
        for (int i = 0; i < m_basketApples.size() && i < 12; ++i) {
            const int ax = bx + 10 + col * 18;
            const int ay = by + 5  + row * 18;
            p.drawPixmap(ax, ay, 16, 16, m_appleSmall);
            ++col;
            if (col >= 6) { col = 0; ++row; }
        }
    } else {
        // Fallback: coloured dots
        p.setBrush(QColor(0xFF4444));
        p.setPen(Qt::NoPen);
        int row = 0, col = 0;
        for (int i = 0; i < m_basketApples.size() && i < 12; ++i) {
            const int ax = bx + 14 + col * 18;
            const int ay = by + 10 + row * 18;
            p.drawEllipse(ax, ay, 12, 12);
            ++col;
            if (col >= 6) { col = 0; ++row; }
        }
    }
}

void AppleWindow::drawApples(QPainter &p)
{
    for (const AppleItem &apple : m_apples) {
        const int r  = kAppleRadius;
        const int ax = static_cast<int>(apple.x) - r;
        const int ay = static_cast<int>(apple.y) - r;

        if (apple.state == AppleItem::State::Caught) {
            // Briefly show a bright flash
            qreal alpha = static_cast<qreal>(apple.animFrame) / 20.0;
            p.setOpacity(alpha);
            p.setBrush(QColor(255, 255, 100));
            p.setPen(Qt::NoPen);
            p.drawEllipse(ax - 4, ay - 4, r * 2 + 8, r * 2 + 8);
            p.setOpacity(1.0);

        } else if (apple.state == AppleItem::State::Broken) {
            qreal alpha = static_cast<qreal>(apple.animFrame) / 25.0;
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
                // Draw apple with gradient
                QRadialGradient radial(apple.x - r / 3, apple.y - r / 3, r);
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
                p.drawLine(static_cast<int>(apple.x),
                           ay,
                           static_cast<int>(apple.x) + 4,
                           ay - 7);
            }

            // Letter label
            p.setPen(Qt::white);
            QFont font(QStringLiteral("Arial"), 16, QFont::Bold);
            p.setFont(font);
            QRect letterRect(ax, ay, r * 2, r * 2);
            // Dark circle behind letter for readability
            p.setBrush(QColor(0, 0, 0, 100));
            p.setPen(Qt::NoPen);
            p.drawEllipse(static_cast<int>(apple.x) - 11,
                          static_cast<int>(apple.y) - 11,
                          22, 22);
            p.setPen(Qt::white);
            p.drawText(letterRect, Qt::AlignCenter, apple.letter);
        }
    }
}

void AppleWindow::drawOverlay(QPainter &p)
{
    // Semi-transparent dark overlay
    p.setBrush(QColor(0, 0, 0, 160));
    p.setPen(Qt::NoPen);
    p.drawRect(rect());

    const QPoint centre(width() / 2, height() / 2);

    // Panel
    QRect panel(centre.x() - 180, centre.y() - 130, 360, 260);
    p.setBrush(QColor(0x1A1A2E));
    p.setPen(QPen(QColor(0xFF6B6B), 2));
    p.drawRoundedRect(panel, 16, 16);

    p.setPen(Qt::white);

    if (m_state == GameState::Paused) {
        p.setFont(QFont(QStringLiteral("Arial"), 28, QFont::Bold));
        p.drawText(panel, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap,
                   QStringLiteral("\n") + tr("Paused"));
        p.setFont(QFont(QStringLiteral("Arial"), 13));
        p.drawText(panel.adjusted(0, 90, 0, 0),
                   Qt::AlignHCenter | Qt::AlignTop,
                   tr("Press Resume to continue"));

    } else { // GameOver
        p.setFont(QFont(QStringLiteral("Arial"), 28, QFont::Bold));
        p.setPen(QColor(0xFF6B6B));
        p.drawText(panel, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap,
                   QStringLiteral("\n") + tr("Game Over"));

        p.setPen(Qt::white);
        p.setFont(QFont(QStringLiteral("Arial"), 14));
        const QString stats =
            tr("Score: %1    Level: %2\nCaught: %3    Missed: %4")
                .arg(m_score).arg(m_level)
                .arg(m_caughtCount).arg(m_missedCount);
        p.drawText(panel.adjusted(0, 100, 0, -60),
                   Qt::AlignCenter | Qt::TextWordWrap, stats);
    }
}
