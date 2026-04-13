#include "applewindow.h"
#include "SaveApple/Model/GameData.h"
#include "SaveApple/View/GameView.h"
#include "SaveApple/Controller/GameController.h"
#include "SaveApple/Config/GameConfig.h"
#include "SaveApple/View/Settingsdialog.h"
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QPixmap>
#include <QIcon>

// ── Constructor ──

AppleWindow::AppleWindow(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle(tr("Save the Apple"));
    setMinimumSize(700, 550);
    resize(800, 620);

    setupMvc();    // 1. 先构建三元组（在调用 setupUi 之前必须已存在 GameView）
    setupUi();     // 2. 创建HUD栏和覆盖按钮
    setupGameButtons();
    setupMusic();
    setCatchSound(); 
    connectSignals();  // 3. 在所有对象创建完成后再进行连接
}


AppleWindow::~AppleWindow()
{
    // 在销毁前断开关联，以确保 GameData 不会对已销毁的对象调用 onUpdate()
    if (m_gameData)
        m_gameData->detach(this);
}


// ── MVC wiring ──
void AppleWindow::setupMvc()
{
    m_gameData   = new GameData();
    m_gameView   = new GameView(m_gameData, this);
    m_controller = new GameController(m_gameData, m_gameView, this);

        // 注册两个观察者：
    //   GameView::onUpdate()    → 安排重绘游戏画布
    //   AppleWindow::onUpdate() → 刷新 Qt 控件 HUD 标签
    m_gameData->attach(m_gameView);
    m_gameData->attach(this);
}

// ── HUD bar + start button ──
void AppleWindow::setupUi()
{
    const int hudH = GameConfig::getInstance().getHudHeight();

    // ── HUD 栏（固定高度，位于 GameView 上方） ──
    QWidget* hud = new QWidget(this);
    hud->setObjectName(QStringLiteral("appleHud"));
    hud->setFixedHeight(hudH);
    hud->move(0, 0);

    QHBoxLayout* hudLayout = new QHBoxLayout(hud);
    hudLayout->setContentsMargins(12, 0, 12, 0);
    hudLayout->setSpacing(20);

    m_backBtn = new QPushButton(tr("◀ Back"), hud);
    m_backBtn->setObjectName(QStringLiteral("hudButton"));
    m_backBtn->setFixedWidth(90);
    connect(m_backBtn, &QPushButton::clicked, this, &QWidget::close);

    // 分数 / 生命值 / 关卡 — Qt 控件标签，并非绘制在画布上
    m_scoreLabel = new QLabel(tr("Score: 0"), hud);
    m_scoreLabel->setObjectName(QStringLiteral("hudLabel"));

    m_livesLabel = new QLabel(tr("Lives: ♥♥♥♥♥"), hud);
    m_livesLabel->setObjectName(QStringLiteral("hudLabel"));

    m_levelLabel = new QLabel(tr("Level: 1"), hud);
    m_levelLabel->setObjectName(QStringLiteral("hudLabel"));

    m_pauseBtn = new QPushButton(tr("Pause"), hud);
    m_pauseBtn->setObjectName(QStringLiteral("hudButton"));
    m_pauseBtn->setFixedWidth(80);
    m_pauseBtn->setVisible(false);  // 游戏开始前隐藏

    hudLayout->addWidget(m_backBtn);
    hudLayout->addStretch();
    hudLayout->addWidget(m_scoreLabel);
    hudLayout->addWidget(m_livesLabel);
    hudLayout->addWidget(m_levelLabel);
    hudLayout->addStretch();
    hudLayout->addWidget(m_pauseBtn);

    // ── GameView 填充 HUD 下方的区域 ──
    // We manually position it; resizeEvent keeps it in sync.
    m_gameView->setGeometry(0, hudH, width(), height() - hudH);
    m_gameView->setFocusPolicy(Qt::StrongFocus);

    // ── 开始 / 重玩按钮（位于 GameView 正上方） ──
    m_startBtn = new QPushButton(tr("Start Game"), this);
    m_startBtn->setObjectName(QStringLiteral("bigButton"));
    m_startBtn->setFixedSize(200, 55);

    // 初始位置 — resizeEvent() 会保持其同步
    repositionStartBtn();
}

// ── Four image buttons (bottom-left) ──
QPushButton* AppleWindow::makeImageButton(const QString& resourcePath,
                                          const QString& fallbackText,
                                          QWidget* parent)
{
    QPushButton* btn = new QPushButton(parent);
    btn->setFlat(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setAttribute(Qt::WA_TranslucentBackground);
    btn->setAttribute(Qt::WA_Hover);   // needed for QIcon::Active hover state

    QPixmap px(resourcePath);
    if (!px.isNull()) {
        // Sprite strip: 3 horizontal frames
        //   frame 0 — Normal  (default appearance)
        //   frame 1 — Hover   (mouse over)
        //   frame 2 — Pressed (mouse down / click)
        const int frameW = px.width() / 3;
 
        // Store all 3 raw (unscaled) frames so repositionGameButtons()
        // can rescale them whenever the window is resized.
        btn->setProperty("sprite0", px.copy(0 * frameW, 0, frameW, px.height()));
        btn->setProperty("sprite1", px.copy(1 * frameW, 0, frameW, px.height()));
        btn->setProperty("sprite2", px.copy(2 * frameW, 0, frameW, px.height()));
        btn->setProperty("hasSprite", true);
 
        // Actual icon size is set in repositionGameButtons(); placeholder here.
        btn->setStyleSheet(
            QStringLiteral("QPushButton { border:none; background:transparent; padding:0; }"
                           "QPushButton:disabled { opacity:0.4; }"));
 
        // Frame 2 (pressed) via pressed / released signals
        // (QIcon::Active covers hover; there's no dedicated "Pressed" mode,
        //  so we swap the icon manually on press/release.)
        connect(btn, &QPushButton::pressed, btn, [btn]() {
            const QPixmap f2 = btn->property("sprite2").value<QPixmap>();
            if (f2.isNull()) return;
            const int s = btn->width();
            QIcon icon;
            icon.addPixmap(f2.scaled(s, s, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            btn->setIcon(icon);
        });
        connect(btn, &QPushButton::released, btn, [btn]() {
            // Restore Normal + Hover frames after release
            const QPixmap f0 = btn->property("sprite0").value<QPixmap>();
            const QPixmap f1 = btn->property("sprite1").value<QPixmap>();
            if (f0.isNull()) return;
            const int s = btn->width();
            QIcon icon;
            icon.addPixmap(f0.scaled(s, s, Qt::IgnoreAspectRatio, Qt::SmoothTransformation),
                           QIcon::Normal);
            if (!f1.isNull())
                icon.addPixmap(f1.scaled(s, s, Qt::IgnoreAspectRatio, Qt::SmoothTransformation),
                               QIcon::Active);
            btn->setIcon(icon);
        });
 
    } else {
        btn->setText(fallbackText);
        btn->setProperty("hasSprite", false);
        btn->setStyleSheet(
            QStringLiteral("QPushButton {"
                           "  background:rgba(42,58,42,200); color:#90ee90;"
                           "  border:2px solid #4a7a4a; border-radius:50%;"
                           "  font-size:10px; font-weight:bold;"
                           "}"
                           "QPushButton:hover   { background:rgba(58,90,58,220); }"
                           "QPushButton:pressed { background:rgba(80,120,80,240); }"
                           "QPushButton:disabled{ color:#556655; border-color:#334433; }"));
    }
    return btn;
}

void AppleWindow::setupGameButtons()
{
    m_imgStartBtn = makeImageButton(
        QStringLiteral(":/images/Common/PUBLIC_START.png"),
        tr("Start"), this);
 
    m_imgPauseBtn = makeImageButton(
        QStringLiteral(":/images/Common/PUBLIC_PAUSE.png"),
        tr("Pause"), this);
 
    m_imgSettingsBtn = makeImageButton(
        QStringLiteral(":/images/Common/PUBLIC_SETUP.png"),
        tr("Setup"), this);
 
    m_imgExitBtn = makeImageButton(
        QStringLiteral(":/images/Common/PUBLIC_END.png"),
        tr("Exit"), this);
 
    repositionGameButtons();
}


// ── 尺寸调整的传播 ──
void AppleWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    const int hudH = GameConfig::getInstance().getHudHeight();

    // HUD 占据全宽
    if (auto* hud = findChild<QWidget*>(QStringLiteral("appleHud")))
        hud->resize(width(), hudH);

    // GameView 负责填充其余部分
    if (m_gameView)
        m_gameView->setGeometry(0, hudH, width(), height() - hudH);

    // 将“开始”按钮置于窗口下半部分中央
    repositionStartBtn();
    repositionGameButtons();
}

// ── 按钮定位辅助 ──
// “开始游戏”按钮位于窗口中央（就绪状态）。
// “重玩”按钮位于“游戏结束”覆盖面板的底部，
// 因此它永远不会与统计信息文本重叠。

void AppleWindow::repositionStartBtn()
{
    if (!m_startBtn) return;
 
    const int hudH = GameConfig::getInstance().getHudHeight();
    const int cx   = (width() - m_startBtn->width()) / 2;
    int y;
 
    if (m_gameData && m_gameData->isGameOver()) {
        const int panelCentreY = hudH + (height() - hudH) / 2;
        y = panelCentreY + 130 - m_startBtn->height() - 15;
    } else {
        y = hudH + (height() - hudH) / 2 + 30;
    }
    m_startBtn->move(cx, y);
}

void AppleWindow::repositionGameButtons()
{
    if (!m_imgStartBtn || !m_gameView) return;
 
    const QVector<QPoint> pawCentres = m_gameView->getPawCenters();
    const QVector<int>    pawRadii   = m_gameView->getPawRadii();
    const int             hudH       = GameConfig::getInstance().getHudHeight();
 
    QPushButton* buttons[4] = {
        m_imgStartBtn, m_imgPauseBtn, m_imgSettingsBtn, m_imgExitBtn
    };
 
    // ── Check all circles are actually inside the visible GameView area ──
    // When the window is made very wide, KeepAspectRatioByExpanding drives
    // scale by width, the image gets much taller than the GameView, and the
    // paw circles (near 95% of image height) are cropped off the bottom.
    const int gvW = m_gameView->width();
    const int gvH = m_gameView->height();
 
    bool allVisible = (pawCentres.size() == 4 && pawRadii.size() == 4
                       && pawRadii[0] > 8);
    for (int i = 0; allVisible && i < 4; ++i) {
        const QPoint& c = pawCentres[i];
        if (c.x() < 0 || c.x() >= gvW || c.y() < 0 || c.y() >= gvH)
            allVisible = false;
    }
 
    auto applySprite = [](QPushButton* btn, int btnSize) {
        const QPixmap f0 = btn->property("sprite0").value<QPixmap>();
        const QPixmap f1 = btn->property("sprite1").value<QPixmap>();
        if (f0.isNull()) return;
        QIcon icon;
        icon.addPixmap(f0.scaled(btnSize, btnSize,
                                 Qt::IgnoreAspectRatio, Qt::SmoothTransformation),
                       QIcon::Normal);
        if (!f1.isNull())
            icon.addPixmap(f1.scaled(btnSize, btnSize,
                                     Qt::IgnoreAspectRatio, Qt::SmoothTransformation),
                           QIcon::Active);
        btn->setIcon(icon);
        btn->setIconSize(QSize(btnSize, btnSize));
    };
 
    if (allVisible) {
        // ── Align each button with its paw circle ──────────────────────
        for (int i = 0; i < 4; ++i) {
            const int btnSize = qMax(20, pawRadii[i] * 2 - 6);
            buttons[i]->setFixedSize(btnSize, btnSize);
            applySprite(buttons[i], btnSize);
 
            // pawCentres are GameView-local; add hudH for AppleWindow coords
            const QPoint centre = pawCentres[i] + QPoint(0, hudH);
            buttons[i]->move(centre.x() - btnSize / 2,
                             centre.y() - btnSize / 2);
        }
    } else {
        // ── Fallback: fixed row at bottom-left of AppleWindow ──────────
        // Use a button size proportional to the window so it stays readable
        const int btnSize = qBound(36, height() / 14, 56);
        const int gap     = 6;
        const int baseX   = 14;
        // Anchor to AppleWindow bottom (not GameView bottom), so the row
        // doesn't drift when hudH is included
        const int baseY   = height() - btnSize - 12;
 
        for (int i = 0; i < 4; ++i) {
            buttons[i]->setFixedSize(btnSize, btnSize);
            applySprite(buttons[i], btnSize);
            buttons[i]->move(baseX + i * (btnSize + gap), baseY);
        }
    }
}

// ── IObserver：每当模型发生变化时刷新HUD标签 ──
void AppleWindow::onUpdate()
{
    m_scoreLabel->setText(tr("Score: %1").arg(m_gameData->getScore()));
 
    const int maxLives = m_gameData->getMaxLives();
    QString hearts;
    for (int i = 0; i < maxLives; ++i)
        hearts += (i < m_gameData->getLives())
                  ? QStringLiteral("♥") : QStringLiteral("♡");
    m_livesLabel->setText(tr("Lives: %1").arg(hearts));
 
    const GameConfig& cfg = GameConfig::getInstance();
    m_levelLabel->setText(
        tr("Level: %1  [%2/%3]")
            .arg(m_gameData->getLevel())
            .arg(m_gameData->getLevelCaught())
            .arg(cfg.getLevelTarget()));
 
    const bool gameActive = m_gameData->isRunning()
                         || m_gameData->isPaused()
                         || m_gameData->isLevelComplete();
 
    m_pauseBtn->setVisible(gameActive);
    m_pauseBtn->setText(m_gameData->isPaused() ? tr("Resume") : tr("Pause"));
    m_startBtn->setVisible(!gameActive);
    m_startBtn->setText(m_gameData->isGameOver() ? tr("Play Again") : tr("Start Game"));
 
    if (m_imgPauseBtn)
        m_imgPauseBtn->setEnabled(m_gameData->isRunning() || m_gameData->isPaused());
    if (m_imgStartBtn)
        m_imgStartBtn->setEnabled(!gameActive || m_gameData->isGameOver());
 
    repositionStartBtn();
}

// ── 信号/插槽连接 ──
void AppleWindow::connectSignals()
{
    connect(m_pauseBtn, &QPushButton::clicked, this, [this]() {
        if (m_gameData->isRunning())     m_controller->pauseGame();
        else if (m_gameData->isPaused()) m_controller->resumeGame();
    });
 
    connect(m_startBtn, &QPushButton::clicked, this, [this]() {
        if (m_gameData->isGameOver())
            m_controller->restartGame();
        m_controller->startGame();
    });
 
    connect(m_gameView, &GameView::keyPressed,
            m_controller, &GameController::handleKeyPress);
 
    connect(m_controller, &GameController::fruitCaught, this, [this]() {
        if (m_catchSound && GameConfig::getInstance().isSoundEnabled())
            m_catchSound->play();
    });
 
    connect(m_imgStartBtn, &QPushButton::clicked, this, [this]() {
        if (m_gameData->isGameOver())
            m_controller->restartGame();
        if (!m_gameData->isRunning() && !m_gameData->isLevelComplete())
            m_controller->startGame();
    });
 
    connect(m_imgPauseBtn, &QPushButton::clicked, this, [this]() {
        if (m_gameData->isRunning())     m_controller->pauseGame();
        else if (m_gameData->isPaused()) m_controller->resumeGame();
    });
 
    connect(m_imgSettingsBtn, &QPushButton::clicked, this, [this]() {
        QTimer::singleShot(80, this, [this]() {
            const bool wasRunning = m_gameData->isRunning();
            if (wasRunning) m_controller->pauseGame();
            AppleSettingsDialog dlg(this);
            dlg.exec();
            if (wasRunning && m_gameData->isPaused()) m_controller->resumeGame();
            m_gameView->setFocus();
        });
    });
 
    connect(m_imgExitBtn, &QPushButton::clicked, this, &QWidget::close);
 
    connect(m_controller, &GameController::gameStateChanged,
            this, &AppleWindow::onUpdate);
}

// ── 背景音乐 ──
void AppleWindow::setupMusic()
{
    m_musicPlayer = new QMediaPlayer(this);
    QAudioOutput* audioOutput = new QAudioOutput(this);
    m_musicPlayer->setAudioOutput(audioOutput);
    audioOutput->setVolume(GameConfig::getInstance().isSoundEnabled() ? 0.2f : 0.0f);

    m_musicPlayer->setSource(
        QUrl(QStringLiteral("qrc:/sounds/Apple/APPLE_BG.wav")));
    m_musicPlayer->setLoops(QMediaPlayer::Infinite);
    m_musicPlayer->play();
}

// ── 捕捉音效 ──
void AppleWindow::setCatchSound()
{
    m_catchSound = new QSoundEffect(this);
    m_catchSound->setSource(
        QUrl(QStringLiteral("qrc:/sounds/Apple/APPLE_IN.wav")));
    m_catchSound->setVolume(0.7f);
    m_catchSound->setLoopCount(0);
}
