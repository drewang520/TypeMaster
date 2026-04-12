#include "applewindow.h"
#include "SaveApple/Model/GameData.h"
#include "SaveApple/View/GameView.h"
#include "SaveApple/Controller/GameController.h"
#include "SaveApple/Config/GameConfig.h"
#include <QHBoxLayout>
#include <QResizeEvent>

// ── Constructor ──

AppleWindow::AppleWindow(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle(tr("Save the Apple"));
    setMinimumSize(700, 550);
    resize(800, 620);

    setupMvc();    // 1. 先构建三元组（在调用 setupUi 之前必须已存在 GameView）
    setupUi();     // 2. 创建HUD栏和覆盖按钮
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

// ── Window chrome ──
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
    if (m_startBtn)
        repositionStartBtn();
}

// ── 按钮定位辅助 ──
// “开始游戏”按钮位于窗口中央（就绪状态）。
// “重玩”按钮位于“游戏结束”覆盖面板的底部，
// 因此它永远不会与统计信息文本重叠。

void AppleWindow::repositionStartBtn()
{
    const int hudH   = GameConfig::getInstance().getHudHeight();
    const int cx     = (width() - m_startBtn->width()) / 2;
 
    int y;
    if (m_gameData && m_gameData->isGameOver()) {
        // 在 GameView 坐标系中的叠加面板：
        //   中心点 = (gameViewHeight / 2)，面板底部 = 中心点 + 130
        // 转换为 AppleWindow 坐标系：加上 hudH
        const int gameViewH    = height() - hudH;
        const int panelCentreY = hudH + gameViewH / 2;   // in AppleWindow coords
        // 将按钮放置在面板底部边缘上方 15 像素处 (panelCentreY + 130)
        y = panelCentreY + 130 - m_startBtn->height() - 15;
    } else {
        // 就绪状态：全屏窗口中央（HUD下方）
        y = hudH + (height() - hudH) / 2 + 30;
    }
 
    m_startBtn->move(cx, y);
}



// ── IObserver：每当模型发生变化时刷新HUD标签 ──
void AppleWindow::onUpdate()
{
    // Score
    m_scoreLabel->setText(tr("Score: %1").arg(m_gameData->getScore()));
 
    // Lives as heart symbols
    const int maxLives = m_gameData->getMaxLives();
    QString hearts;
    for (int i = 0; i < maxLives; ++i)
        hearts += (i < m_gameData->getLives())
                  ? QStringLiteral("♥") : QStringLiteral("♡");
    m_livesLabel->setText(tr("Lives: %1").arg(hearts));
 
    // Level
    m_levelLabel->setText(tr("Level: %1").arg(m_gameData->getLevel()));
 
    // Button visibility / text
    const bool gameActive = m_gameData->isRunning() || m_gameData->isPaused();
    m_pauseBtn->setVisible(gameActive);
    m_pauseBtn->setText(m_gameData->isPaused() ? tr("Resume") : tr("Pause"));
    m_startBtn->setVisible(!gameActive);
    m_startBtn->setText(m_gameData->isGameOver() ? tr("Play Again") : tr("Start Game"));

    // 每次状态变化时重新定位：“再次播放”移至
    // 覆盖面板内；“开始游戏”移至窗口中心。
    repositionStartBtn();
}



// ── 信号/插槽连接 ──
void AppleWindow::connectSignals()
{
    // 暂停按钮
    connect(m_pauseBtn, &QPushButton::clicked, this, [this]() {
        if (m_gameData->isRunning())
            m_controller->pauseGame();
        else if (m_gameData->isPaused())
            m_controller->resumeGame();
    });

    // 开始 / 重播按钮
    connect(m_startBtn, &QPushButton::clicked, this, [this]() {
        if (m_gameData->isGameOver())
            m_controller->restartGame();
        m_controller->startGame();
    });

    // ── 错误修复 1：连接 GameView 与 GameController 之间的按键事件 ──────────
    // 之前完全缺少这一部分——这就是按键没有反应的原因。
    connect(m_gameView, &GameView::keyPressed,
            m_controller, &GameController::handleKeyPress);

    // 接住水果时播放接球音效
    connect(m_controller, &GameController::fruitCaught, this, [this]() {
        if (m_catchSound)
            m_catchSound->play();
    });
}

// ── 背景音乐 ──
void AppleWindow::setupMusic()
{
    m_musicPlayer = new QMediaPlayer(this);

    QAudioOutput* audioOutput = new QAudioOutput(this);
    m_musicPlayer->setAudioOutput(audioOutput);
    audioOutput->setVolume(0.2f);

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
